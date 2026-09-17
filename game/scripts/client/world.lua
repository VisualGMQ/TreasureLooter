local World = require("common.world")

local k_player_client_script = TL_Common.Path("scripts/client/behaviors/net_player_behavior.lua")
local k_player_client_replicate_script = TL_Common.Path("scripts/client/behaviors/net_player_replicate_behavior.lua")

-- UI entity names, see game.scene.xml / net_test_welcome.scene.xml.
local k_ui_lobby_text = "LobbyText"
local k_ui_server_full_text = "ServerFullText"
local k_ui_return_title_button = "ReturnTitleButton"
local k_ui_dead_message = "DeadMessage"
local k_ui_win_message = "WinMessage"
local k_ui_countdown_text = "CountdownText"

local k_game_scene_path = "assets/gpa/scenes/game.scene.xml"
local k_welcome_scene_path = "assets/gpa/scenes/net_test_welcome.scene.xml"

-- Start countdown: the numbers tick once per second, then the banner replaces
-- them for one more second (see `startCountdownUI`).
local k_countdown_start_text = "START!"
local k_countdown_tick_interval = 1

-- "You are here" arrow spawned over the local player during the countdown, see
-- assets/gpa/objects/player_arrow.prefab.xml. The position is relative to the
-- player it is parented to.
local k_player_arrow_prefab = "assets/gpa/objects/player_arrow.prefab.xml"
local k_player_arrow_offset = TL_Common.Vec2(0, -14)

---@class FXEntry
---@field entity LogicEntity
---@field gameobject any
---@field anim_end_event_id EventListenerID

--- One `SpawnPlayerReply` kept back while the client is still in the lobby: the
--- character is never created in the welcome scene, only once the game scene
--- and its level are ready (see `flushPendingSpawns`).
---@class PendingSpawn
---@field m_net_id NetID
---@field m_did DID
---@field m_position Vec2

---@class ClientWorld : World
---@field m_player_hint LogicEntity
---@field private _interp_delay number
---@field private _net_gameobjects table<NetID, ClientGameObject>
---@field private _idle_fx_pool FXEntry[]
---@field private _fx_entries FXEntry[]
---@field private _pending_spawns PendingSpawn[]
---@field private _dead_net_ids table<NetID, boolean>
---@field private _level_ready boolean
---@field private _game_scene_entered boolean
---@field private _local_net_id NetID
---@field private _control_locked boolean
---@field private _countdown_timer Timer?
---@field private _return_timer Timer?
---@field private _countdown_text_tick number
---@field private _player_arrow LogicEntity
---@field private _game_over boolean
---@field private _winner_net_id number
---@field private _local_hp number
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = World })

local k_heart_names = { "Heart1", "Heart2", "Heart3" }
local k_max_hearts = #k_heart_names

---@return ClientWorld
function _M.new()
    local self = World.new()
    ---@cast self ClientWorld
    self.m_player_hint = TL_Common.null_entity
    self._idle_fx_pool = {}
    self._fx_entries = {}
    self._net_gameobjects = {}
    self._pending_spawns = {}
    self._dead_net_ids = {}
    self._level_ready = false
    self._game_scene_entered = false
    self._local_net_id = 0
    self._control_locked = true
    self._countdown_timer = nil
    self._return_timer = nil
    self._countdown_text_tick = 0
    self._player_arrow = TL_Common.null_entity
    self._interp_delay = 1.5 / TL_Common.GetContext():GetCommonConfig().m_server_fps
    self._game_over = false
    self._winner_net_id = 0
    self._local_hp = 0
    return setmetatable(self, _M)
end

--- The local peer id, or 0 when there is no valid connection yet. The live peer
--- id is preferred, the one recorded from our own `SpawnPlayerReply` is the
--- fallback (it survives the disconnection the server does after `Win`).
---@return number
function _M:GetLocalNetID()
    local ctx = TL_Client.GetContext()
    local peer = ctx:GetNetPeer()
    if peer and peer:IsValid() then
        return peer:GetID()
    end
    return self._local_net_id
end

--- Whether the local player input is currently ignored. It stays locked from
--- the lobby until the server countdown (`PrepareCountdown`) has elapsed.
---@return boolean
function _M:IsControlLocked()
    return self._control_locked
end

--- Whether the round is over (a `Win` message has been received). The local
--- player must not send any more input once it is set.
---@return boolean
function _M:IsGameOver()
    return self._game_over
end

---@return NetID
function _M:GetWinnerNetID()
    return self._winner_net_id
end

--- Look up an entity by name under the current scene's UI root. Returns
--- `null_entity` when the scene or the entity is missing (the assets may not be
--- ready yet).
---@param name string
---@return LogicEntity
function _M:GetUIEntity(name)
    local ctx = TL_Client.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return TL_Common.null_entity
    end

    return ctx:GetEntityNameManager():FindChildByName(scene:GetUIRootEntity(), name)
end

---@param name string
---@return UIWidget?
function _M:GetUIWidget(name)
    local entity = self:GetUIEntity(name)
    if entity == nil or entity == TL_Common.null_entity then
        return nil
    end

    return TL_Client.GetContext():GetUIManager():Get(entity)
end

---@param name string
---@param enable boolean
function _M:SetUIEnableDraw(name, enable)
    local widget = self:GetUIWidget(name)
    if widget then
        widget.m_enable_draw = enable
    end
end

--- Replace the text of a UI widget. No-op when the widget or its text
--- component is missing.
---@param name string
---@param text string
function _M:SetUIText(name, text)
    local widget = self:GetUIWidget(name)
    if not widget then
        return
    end

    local ui_text = widget:GetText()
    if ui_text then
        ui_text:ChangeText(text)
    end
end

--- Reload the heart widgets from `_local_hp`: heart `i` is shown when the
--- local player still has at least `i` hp.
function _M:RefreshHearts()
    for i = 1, k_max_hearts do
        self:SetUIEnableDraw(k_heart_names[i], i <= self._local_hp)
    end
end

--- Record the local player's hp and refresh the hearts. Ignores unchanged
--- values so the widget lookup only happens when the hp actually changes. The
--- "return to title" button appears as soon as the local player is dead.
---@param hp number
function _M:SetLocalHp(hp)
    if self._local_hp == hp then
        return
    end
    self._local_hp = hp
    self:RefreshHearts()
    if hp <= 0 then
        self:SetUIEnableDraw(k_ui_return_title_button, true)
    end
end

function _M:GetNetInterpDelay()
    return self._interp_delay
end

---@param ctx any
---@param did DID
---@param local_pos Vec2
---@param object_definitions any
function _M:reuseFX(ctx, did, local_pos, object_definitions)
    local entry = self._idle_fx_pool[#self._idle_fx_pool]
    table.remove(self._idle_fx_pool)

    ctx:GetSpriteManager():Enable(entry.entity)
    entry.gameobject.m_transform.m_position = local_pos

    if entry.anim_end_event_id and entry.anim_end_event_id ~= TL_Common.null_event_listener_id then
        ctx:GetEventSystem():Remove(entry.anim_end_event_id)
    end

    local fx_definition = object_definitions:GetFX(did)
    if fx_definition then
        local anim_player = ctx:GetAnimationPlayerManager():Get(entry.entity, 0)
        if anim_player then
            anim_player:ChangeAnimation(fx_definition.m_animation.m_animation)
        end
    end

    local anim_player = ctx:GetAnimationPlayerManager():Get(entry.entity, 0)
    if anim_player then
        -- The animation player belongs to the scene and becomes dangling once
        -- the scene is unloaded, while this listener is still registered for a
        -- frame. Only its numeric id is captured, never the player itself.
        local anim_player_id = anim_player:GetID()
        entry.anim_end_event_id = ctx:GetEventSystem():AddAnimationEndEvent(
            function(id, event)
                if event:GetAnimationPlayerID() ~= anim_player_id then
                    return
                end
                -- The entity can be destroyed with the scene while the event is
                -- still in flight: never touch a gameobject that is gone.
                if not ctx:GetTransformManager():Has(entry.gameobject:GetEntity()) then
                    return
                end
                if entry.gameobject.m_fx_component then
                    entry.gameobject.m_fx_component:Disable()
                end
                table.insert(self._idle_fx_pool, entry)
            end)
    else
        entry.anim_end_event_id = TL_Common.null_event_listener_id
    end
    if entry.gameobject.m_fx_component and anim_player then
        entry.gameobject.m_fx_component:Play()
    end
end

---@param ctx any
---@param did DID
---@param local_pos Vec2
---@param object_definitions any
function _M:createFX(ctx, did, local_pos, object_definitions)
    local scene = ctx:GetSceneManager():GetCurrentScene()

    local spawn_info = TL_Schema.ObjectSpawnDefinition()
    spawn_info.m_did = did

    local ClientCreation = require("client.creation")
    local entity = ClientCreation.CreateFX(ClientCreation, scene, spawn_info, local_pos, object_definitions)

    local land_relationship = ctx:GetRelationshipManager():Get(self.m_land_entity)
    if land_relationship then
        local item_relationship = ctx:GetRelationshipManager():Get(entity)
        if item_relationship then item_relationship:RemoveFromParent() end
        land_relationship:AddChild(entity)
    end

    local script = ctx:GetScriptManager():Get(entity)
    if not script or not script.m_gameobject then
        ctx:Log("client: create FX without a gameobject")
        return
    end

    local gameobject = script.m_gameobject
    local new_entry = { entity = entity, gameobject = gameobject, anim_end_event_id = TL_Common.null_event_listener_id }
    table.insert(self._fx_entries, new_entry)

    local anim_player = ctx:GetAnimationPlayerManager():Get(entity, 0)
    if anim_player then
        -- Only the numeric id is captured: the player is destroyed with the
        -- scene while the listener may outlive a frame (see `reuseFX`).
        local anim_player_id = anim_player:GetID()
        new_entry.anim_end_event_id = ctx:GetEventSystem():AddAnimationEndEvent(
            function(id, event)
                if event:GetAnimationPlayerID() ~= anim_player_id then
                    return
                end
                if not ctx:GetTransformManager():Has(gameobject:GetEntity()) then
                    return
                end
                if gameobject.m_fx_component then
                    gameobject.m_fx_component:Disable()
                end
                table.insert(self._idle_fx_pool, new_entry)
            end)
    end
    if gameobject.m_fx_component and anim_player then
        gameobject.m_fx_component:Play()
    end
end

---@param did DID
---@param position Vec2
function _M:AddFX(did, position)
    local ctx = TL_Client.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then return end

    local object_definitions = self.m_object_definitions
    if not object_definitions then return end

    local land_transform = ctx:GetTransformManager():Get(self.m_land_entity)
    local local_pos = position
    if land_transform then
        local_pos = position - land_transform:GetGlobalPosition()
    end

    if #self._idle_fx_pool > 0 then
        self:reuseFX(ctx, did, local_pos, object_definitions)
    else
        self:createFX(ctx, did, local_pos, object_definitions)
    end
end

function _M:RegisterNetEventHandler()
    local ctx = TL_Client.GetContext()
    local event_system = ctx:GetEventSystem()
    event_system:AddNetMsg_SpawnPlayerReplyEvent(function(id, peer, reply)
        self:onSpawnPlayerReply(reply)
    end)
    event_system:AddNetMsg_ConnectEvent(function(id, peer, connect)
        self:onNetConnect(peer, connect)
    end)
    -- The client only ever has one peer (the server): a disconnect always means
    -- the local connection is gone.
    event_system:AddNetMsg_DisconnectEvent(function(id, peer, disconnect)
        self:onNetDisconnect(peer)
    end)
    event_system:AddNetMsg_PlayerJoinedEvent(function(id, peer, payload)
        self:onPlayerJoined(payload)
    end)
    event_system:AddNetMsg_ServerFullEvent(function(id, peer, payload)
        self:onServerFull(payload)
    end)
    event_system:AddNetMsg_PrepareCountdownEvent(function(id, peer, payload)
        self:onPrepareCountdown(payload)
    end)
    event_system:AddNetMsg_KillEvent(function(id, peer, msg)
        self:onPlayerKill(msg)
    end)
    event_system:AddNetMsg_HurtEvent(function(id, peer, msg)
        self:onPlayerHurt(msg)
    end)
    event_system:AddNetMsg_WinEvent(function(id, peer, msg)
        self:onPlayerWin(msg)
    end)
    event_system:AddUIMouseClickedEvent(function(id, event)
        self:onUIClicked(event)
    end)
end

--- The lobby counter changed. The game scene is loaded only once the lobby is
--- full: the server creates the characters at that moment, so the client waits
--- (with the characters it already knows about buffered) instead of switching
--- on connection.
---@param payload ProtoPlayerJoined
---@private
function _M:onPlayerJoined(payload)
    local ctx = TL_Client.GetContext()
    local count = payload:m_count()
    local max = payload:m_max()

    -- The "full" broadcast is sent once when the last player joins and once more
    -- when the game starts: switch only the first time.
    if count >= max then
        self:switchToGameScene()
        return
    end

    self:SetUIEnableDraw(k_ui_lobby_text, true)
    self:SetUIText(k_ui_lobby_text, string.format("等待他人加入中%d/%d", count, max))
    ctx:Log("client lobby: ", count, "/", max)
end

--- This connection was rejected because the lobby is already full. The server
--- disconnects the peer right after, the title screen stays as it is.
---@param payload ProtoServerFull
---@private
function _M:onServerFull(payload)
    TL_Client.GetContext():Log("client rejected: server full")
    self:SetUIEnableDraw(k_ui_server_full_text, true)
end

--- The server armed the start countdown: show it, mark the local player and
--- ignore its input until the `START!` banner is over. The server ignores the
--- inputs during the same window.
---@param payload ProtoPrepareCountdown
---@private
function _M:onPrepareCountdown(payload)
    local seconds = payload:m_countdown()
    local ctx = TL_Client.GetContext()
    ctx:Log("client countdown: ", seconds)

    -- A countdown can be re-sent; drop the previous display before starting a
    -- new one so the text and the arrow never show twice.
    self:stopCountdownTimer()
    self:hideCountdownUI()

    if seconds <= 0 then
        self._control_locked = false
        return
    end

    self._control_locked = true
    self:startCountdownUI(seconds)
    self:showLocalPlayerArrow()
end

--- Show `CountdownText` with the remaining whole seconds. The widget is driven
--- by a one shot 1 second timer restarted on every tick, so no per frame update
--- is needed (see `onCountdownTick`).
---@private
---@param seconds number
function _M:startCountdownUI(seconds)
    self._countdown_text_tick = math.ceil(seconds)
    self:SetUIEnableDraw(k_ui_countdown_text, true)
    self:SetUIText(k_ui_countdown_text, tostring(self._countdown_text_tick))

    local timer = TL_Client.GetContext():GetTimerManager():Create(
        k_countdown_tick_interval, TL_Schema.TimerEventType.Cooldown, 0)
    timer:SetTimerListener(function(event)
        self:onCountdownTick(timer)
    end)
    timer:Start()
    self._countdown_timer = timer
end

--- Hide the countdown text and drop the arrow. Called when the countdown is
--- over (including the `START!` second) and when a new one replaces it.
---@private
function _M:hideCountdownUI()
    self:SetUIEnableDraw(k_ui_countdown_text, false)
    self:destroyLocalPlayerArrow()
end

--- One second of the countdown elapsed. `_countdown_text_tick` counts down
--- from `ceil(seconds)` to `1`; `0` is the `START!` banner and the tick after
--- it ends the countdown, hides the widget and gives the control back.
---@private
---@param timer Timer
function _M:onCountdownTick(timer)
    local tick = self._countdown_text_tick
    if tick > 1 then
        self._countdown_text_tick = tick - 1
        self:SetUIText(k_ui_countdown_text, tostring(tick - 1))
        timer:Start()
        return
    end

    if tick == 1 then
        -- The last number elapsed: keep the banner up for one more second.
        self._countdown_text_tick = 0
        self:SetUIText(k_ui_countdown_text, k_countdown_start_text)
        timer:Start()
        return
    end

    self:hideCountdownUI()
    if self._countdown_timer == timer then
        self._countdown_timer = nil
    end
    self._control_locked = false
end

---@private
function _M:stopCountdownTimer()
    if not self._countdown_timer then
        return
    end

    TL_Client.GetContext():GetTimerManager():Remove(self._countdown_timer)
    self._countdown_timer = nil
end

--- The entity of the local player, or `null_entity` while it does not exist
--- yet (the character is created once the game scene is loaded).
---@private
---@return LogicEntity
function _M:getLocalPlayerEntity()
    local net_id = self:GetLocalNetID()
    if net_id == 0 then
        return TL_Common.null_entity
    end

    local gameobject = self._net_gameobjects[net_id]
    if not gameobject then
        return TL_Common.null_entity
    end
    return gameobject:GetEntity()
end

--- Whether the arrow entity is still alive in the current scene.
---@private
---@return boolean
function _M:hasLocalPlayerArrow()
    if self._player_arrow == TL_Common.null_entity then
        return false
    end

    return TL_Client.GetContext():GetTransformManager():Has(self._player_arrow)
end

--- Parent the arrow prefab over the local player. Idempotent: a second call
--- while the arrow is alive does nothing. The assets may not be ready yet, in
--- which case only the arrow is skipped.
---@private
function _M:showLocalPlayerArrow()
    if self:hasLocalPlayerArrow() then
        return
    end
    self._player_arrow = TL_Common.null_entity

    local player_entity = self:getLocalPlayerEntity()
    if player_entity == TL_Common.null_entity then
        return
    end

    local ctx = TL_Client.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return
    end

    local prefab = ctx:GetAssetsManager():GetPrefabManager():Load(
        k_player_arrow_prefab)
    if not prefab:IsValid() then
        ctx:Log("client: can't load ", k_player_arrow_prefab)
        return
    end

    local transform = TL_Common.Transform()
    transform.m_position = k_player_arrow_offset
    local entity = scene:Instantiate(prefab, transform)
    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    -- `Instantiate` parents the entity to the scene root, move it under the
    -- player: its position is then kept relative to the player.
    local relationship = ctx:GetRelationshipManager():Get(entity)
    if relationship then
        local player_relationship = ctx:GetRelationshipManager():Get(
            player_entity)
        relationship:RemoveFromParent()
        if player_relationship then
            player_relationship:AddChild(entity)
        end
    end

    self._player_arrow = entity
end

--- Destroy the arrow and forget it. The entity can already be gone (destroyed
--- with the player or by a scene switch), in which case only the reference is
--- dropped.
---@private
function _M:destroyLocalPlayerArrow()
    local entity = self._player_arrow
    self._player_arrow = TL_Common.null_entity
    if entity == TL_Common.null_entity then
        return
    end

    local ctx = TL_Client.GetContext()
    if not ctx:GetTransformManager():Has(entity) then
        return
    end

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if scene then
        scene:RemoveEntity(entity)
    end
end

--- A click anywhere in the UI. The button entity exists while hidden, so the
--- click is only honoured when the button is actually drawn.
---@param event UIMouseClickedEvent
---@private
function _M:onUIClicked(event)
    local entity = self:GetUIEntity(k_ui_return_title_button)
    if entity == nil or entity == TL_Common.null_entity then
        return
    end
    if event.m_entity ~= entity then
        return
    end

    local widget = self:GetUIWidget(k_ui_return_title_button)
    if not widget or not widget.m_enable_draw then
        return
    end

    self:returnToTitle()
end

function _M:onNetConnect(peer, connect)
    local ctx = TL_Client.GetContext()
    local request = TL_Proto.SpawnPlayerRequest()
    request:set_m_entity(0)
    request:set_m_did(TL_Schema.DID.CharacterDID1)

    local msg = TL_Proto.NetMsg()
    msg:set_m_spawn_player_request(request)

    local host = ctx:GetNetHost()
    local peer = ctx:GetNetPeer()
    if not host or not peer or not peer:IsValid() then
        return
    end

    host:Send(peer, msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
    ctx:Log("client sent SpawnPlayerRequest")
end

--- The connection to the server is gone (a network drop, a rejection, or the
--- disconnect the server does at the end of the round). The client keeps the
--- current scene, but the local player is not controllable anymore: the
--- countdown must not hand the control back and the player behavior must stop
--- sending to the dead peer. Only the existing "return to title" button is
--- shown, so a round that is not over can still be left.
---@param peer UDPPeer
---@private
function _M:onNetDisconnect(peer)
    local ctx = TL_Client.GetContext()
    ctx:Log("client disconnected from server, peer ", peer and peer:GetID() or -1)

    self:stopCountdownTimer()
    self:hideCountdownUI()
    self._control_locked = true
    -- The button only exists in the game scene; in the title it is a no-op.
    self:SetUIEnableDraw(k_ui_return_title_button, true)
end

---@param kill ProtoKill
function _M:onPlayerKill(kill)
    local net_id = kill:m_net_id()

    -- Remember the player as gone: a late `SpawnPlayerReply` for it (a
    -- disconnect before the match started) must not create a ghost character.
    self._dead_net_ids[net_id] = true
    self:removePendingSpawn(net_id)

    if net_id == self:GetLocalNetID() then
        self:SetLocalHp(0)
        self:SetUIEnableDraw(k_ui_dead_message, true)
        -- The local player died: stop the countdown so a late tick cannot give
        -- the control back, and drop the arrow parented to the dead player.
        self:stopCountdownTimer()
        self:hideCountdownUI()
    end

    local gameobject = self._net_gameobjects[net_id]
    if gameobject == nil then
        return
    end

    -- The entity can already be destroyed (the death animation or a scene
    -- switch): only drop the reference, never touch a freed character.
    if not TL_Client.GetContext():GetTransformManager():Has(gameobject:GetEntity()) then
        self._net_gameobjects[net_id] = nil
        return
    end

    -- The entity keeps living until the death animation ends (the hp component
    -- defers the destroy), it is only removed from the net lookup table.
    if gameobject.m_hp_component then
        gameobject.m_hp_component:ForceDead()
    else
        gameobject:Destroy()
    end
    self._net_gameobjects[net_id] = nil
end

---@param hurt ProtoHurt
function _M:onPlayerHurt(hurt)
    local net_id = hurt:m_net_id()
    local hp = hurt:m_hp()

    if net_id == self:GetLocalNetID() then
        self:SetLocalHp(hp)
        if hp <= 0 then
            self:SetUIEnableDraw(k_ui_dead_message, true)
        end
    end

    local gameobject = self._net_gameobjects[net_id]
    if gameobject == nil or gameobject.m_hp_component == nil then
        return
    end

    -- Never touch a character whose entity is already gone.
    if not TL_Client.GetContext():GetTransformManager():Has(gameobject:GetEntity()) then
        return
    end

    gameobject.m_hp_component:SyncHurt(hp)
end

---@param win ProtoWin
function _M:onPlayerWin(win)
    local net_id = win:m_net_id()
    self._game_over = true
    self._winner_net_id = net_id

    -- the countdown is over for sure, the round is decided: stop it so a late
    -- tick cannot give the control back after the round ended
    self:stopCountdownTimer()
    self:hideCountdownUI()

    if net_id == self:GetLocalNetID() then
        self:SetUIEnableDraw(k_ui_win_message, true)
    end

    -- The server drops every client a few seconds later; the local client only
    -- keeps showing the final frame until the player goes back to the title.
    self:SetUIEnableDraw(k_ui_return_title_button, true)
end

--- The server sends one reply per player when the match starts (and one to the
--- peer right after it joined the lobby). The character is never created while
--- the client is still in the welcome scene: the reply is buffered and created
--- by `flushPendingSpawns` once the game scene's level is ready.
---@param reply ProtoSpawnPlayerReply
---@private
function _M:onSpawnPlayerReply(reply)
    local ctx = TL_Client.GetContext()

    local net_id = reply:m_net_id()

    -- A player that already left (Kill received, or a disconnect before the
    -- match started) must never be recreated by a late reply.
    if self._dead_net_ids[net_id] then
        ctx:Log("client ignores spawn reply of left player, net_id ", net_id)
        return
    end

    local net_position = reply:m_position()
    local pending = {
        m_net_id = net_id,
        m_did = reply:m_did(),
        m_position = TL_Common.Vec2(net_position:m_x(), net_position:m_y()),
    }

    local peer = ctx:GetNetPeer()
    if peer and peer:IsValid() and peer:GetID() == net_id then
        self._local_net_id = net_id
    end

    -- The server announces a player both when it joins and when the match
    -- starts, so the same player can arrive twice; a duplicate would create a
    -- second character for it.
    if self:HasPlayer(net_id) then
        return
    end

    if self._level_ready then
        self:createCharacter(pending)
        return
    end

    table.insert(self._pending_spawns, pending)
    ctx:Log("client buffered player spawn, net_id ", net_id)
end

--- Whether a character for this player already exists (or is waiting to be
--- created).
---@param net_id NetID
---@return boolean
function _M:HasPlayer(net_id)
    if self._net_gameobjects[net_id] then
        return true
    end

    for _, pending in ipairs(self._pending_spawns) do
        if pending.m_net_id == net_id then
            return true
        end
    end
    return false
end

--- Drop the buffered spawn of a player that left the lobby. Called on `Kill`
--- (the server broadcasts one for every disconnect, even before the match
--- started) so `flushPendingSpawns` never creates a ghost character from it.
---@param net_id NetID
---@private
function _M:removePendingSpawn(net_id)
    for i = #self._pending_spawns, 1, -1 do
        if self._pending_spawns[i].m_net_id == net_id then
            table.remove(self._pending_spawns, i)
        end
    end
end

--- Create every character that arrived while the client was still in the lobby.
--- The game scene script calls this once the level is loaded (the level
--- definition and the object definitions must be set, see
--- `ClientGameEntry:OnInit`).
---@return number
function _M:flushPendingSpawns()
    if self._level_ready then
        return 0
    end
    self._level_ready = true

    local count = 0
    for _, pending in ipairs(self._pending_spawns) do
        if self:createCharacter(pending) ~= TL_Common.null_entity then
            count = count + 1
        end
    end
    self._pending_spawns = {}
    TL_Client.GetContext():Log("client created ", count, " postponed player(s)")
    return count
end

--- Tell the server this client loaded the game scene. The server arms the start
--- countdown once every joined player is ready.
function _M:sendPrepareGameFinish()
    local ctx = TL_Client.GetContext()
    local host = ctx:GetNetHost()
    local peer = ctx:GetNetPeer()
    if not host or not peer or not peer:IsValid() then
        return
    end

    local msg = TL_Proto.NetMsg()
    msg:set_m_prepare_game_finish(TL_Proto.PrepareGameFinish())
    host:Send(peer, msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
    ctx:Log("client sent PrepareGameFinish")
end

--- Create a character from a (buffered or just received) `SpawnPlayerReply`. The
--- local player gets the controllable behavior and the level HFSM, every other
--- player the replication behavior.
---@private
---@param pending PendingSpawn
---@return LogicEntity
function _M:createCharacter(pending)
    local ctx = TL_Client.GetContext()

    -- A player that left (Kill) must not be created from a buffered spawn.
    if self._dead_net_ids[pending.m_net_id] then
        ctx:Log("client skips left player, net_id ", pending.m_net_id)
        return TL_Common.null_entity
    end

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return TL_Common.null_entity
    end

    -- lazy require: client.creation requires client.world at load time
    local ClientCreation = require("client.creation")

    local spawn_info = TL_Schema.ObjectSpawnDefinition()
    spawn_info.m_did = pending.m_did

    local hfsm_definition = nil
    if pending.m_net_id == self:GetLocalNetID() then
        spawn_info.m_client_script = k_player_client_script
        local hfsm_path = self.m_level_definition and self.m_level_definition.m_player_related_definition.m_hfsm
        if hfsm_path and not hfsm_path:empty() then
            hfsm_definition = ctx:GetAssetsManager():GetScriptHFSMDefinitionManager():Load(hfsm_path)
        end
    else
        spawn_info.m_client_script = k_player_client_replicate_script
    end

    local entity, gameobject = ClientCreation.CreateCharacter(ClientCreation, scene, spawn_info,
                            pending.m_position, pending.m_net_id, self.m_object_definitions, hfsm_definition)
    if entity == TL_Common.null_entity then
        ctx:Log("client failed to spawn player, net_id ", pending.m_net_id)
        return TL_Common.null_entity
    end

    local root_relationship = ctx:GetRelationshipManager():Get(scene:GetRootEntity())
    if root_relationship then
        root_relationship:AddChild(entity)
    end

    if self._net_gameobjects[pending.m_net_id] ~= nil then
        ctx:Log("client spawned duplicate! net_id: ", pending.m_net_id, ", did ", pending.m_did)
    end
    self._net_gameobjects[pending.m_net_id] = gameobject
    ctx:Log("client spawned player, net_id ", pending.m_net_id, " did ", pending.m_did)
    return entity
end

--- Switch to the game scene once, when the lobby gets full.
---@private
function _M:switchToGameScene()
    if self._game_scene_entered then
        return
    end

    if self:switchToScene(k_game_scene_path) then
        self._game_scene_entered = true
    end
end

--- Load and switch to a scene. A failed load leaves the current scene as it is.
---@private
---@param path string
---@return boolean
function _M:switchToScene(path)
    local ctx = TL_Client.GetContext()
    local scene_manager = ctx:GetSceneManager()
    local handle = scene_manager:Load(path)
    if not handle or not handle:IsValid() then
        ctx:Log("client: load scene failed: ", path)
        return false
    end

    scene_manager:Switch(handle)
    ctx:Log("client switched to scene ", path)
    return true
end

--- Leave the finished round: back to the title screen and a clean client state
--- so a new connection can start another round. The entities of the game scene
--- are destroyed by the scene switch itself.
---
--- The switch is deferred to the next timer tick: this is called from a UI
--- click, i.e. from inside the event dispatch, and switching the scene there
--- destroys (and immediately unloads) the widgets that are still being
--- dispatched.
---@private
function _M:returnToTitle()
    TL_Client.GetContext():Log("client returns to title")

    if self._return_timer then
        return
    end

    local manager = TL_Client.GetContext():GetTimerManager()
    local timer =
        manager:Create(0.01, TL_Schema.TimerEventType.Cooldown, 0)
    timer:SetTimerListener(function(event)
        self:doReturnToTitle()
    end)
    timer:SetTimerStopListener(function(event)
        self._return_timer = nil
    end)
    timer:Start()
    self._return_timer = timer
end

--- The deferred body of `returnToTitle`, run outside of the event dispatch.
---@private
function _M:doReturnToTitle()
    self:stopCountdownTimer()

    if not self:switchToScene(k_welcome_scene_path) then
        return
    end

    -- The camera followed the character through the match and still holds the
    -- level boundary; the title scene lays its world sprites out around the
    -- origin. The boundary has to be replaced *before* moving: MoveTo clamps
    -- the camera into the boundary it currently has.
    local camera = TL_Client.GetContext():GetCamera()
    local boundary = TL_Schema.Rect()
    boundary.m_center = TL_Common.Vec2.ZERO
    boundary.m_half_size = TL_Common.Vec2(100000, 100000)
    camera:SetBoundary(boundary)
    camera:MoveTo(TL_Common.Vec2.ZERO)

    self:resetForNewRound()
end

--- Called when the game scene is unloaded (`ClientGameEntry:OnQuit`). The
--- per-round timers belong to that scene: leaving them running would fire
--- their listeners with entities that no longer exist. The timers are only
--- stopped, never removed here: this runs from the scene switch, which can be
--- inside the dispatch of one of those very timers (removing a timer while it
--- is being dispatched would leave a dangling `this` in the update loop).
function _M:onGameSceneQuit()
    self:stopCountdownTimer()

    if self._return_timer then
        self._return_timer:Stop()
        self._return_timer = nil
    end

    self:hideCountdownUI()
    self._player_arrow = TL_Common.null_entity
    self:clearFXListeners()
end

--- Remove the animation-end listeners of the FX entities. They are registered
--- on the context-wide event system, so they outlive the scene: without this
--- the next round's animation events would call back into a gameobject (and an
--- animation player) destroyed with the previous scene.
---@private
function _M:clearFXListeners()
    local events = TL_Client.GetContext():GetEventSystem()
    for _, entry in ipairs(self._fx_entries) do
        if entry.anim_end_event_id and entry.anim_end_event_id ~= TL_Common.null_event_listener_id then
            events:Remove(entry.anim_end_event_id)
            entry.anim_end_event_id = TL_Common.null_event_listener_id
        end
    end

    self._fx_entries = {}
    self._idle_fx_pool = {}
end

--- Drop every per-round state. The world instance and its net listeners are
--- intentionally kept: the lobby script only re-registers its own listeners.
---@private
function _M:resetForNewRound()
    self._net_gameobjects = {}
    self._pending_spawns = {}
    self._idle_fx_pool = {}
    self._fx_entries = {}
    self._dead_net_ids = {}
    self.m_player_hint = TL_Common.null_entity
    self._level_ready = false
    self._game_scene_entered = false
    self._local_net_id = 0
    self._control_locked = true
    self._game_over = false
    self._winner_net_id = 0
    self._local_hp = 0

    self:SetUIEnableDraw(k_ui_dead_message, false)
    self:SetUIEnableDraw(k_ui_win_message, false)
    self:SetUIEnableDraw(k_ui_return_title_button, false)
    self:SetUIEnableDraw(k_ui_lobby_text, false)
    self:SetUIEnableDraw(k_ui_server_full_text, false)
    -- the arrow and the countdown text belonged to the game scene, which is
    -- already unloaded: only forget them and make sure the next round starts
    -- with a hidden text
    self._countdown_text_tick = 0
    self._player_arrow = TL_Common.null_entity
    self:SetUIEnableDraw(k_ui_countdown_text, false)
end

return _M
