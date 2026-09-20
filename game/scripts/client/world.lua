local World = require("common.world")

---@class FXEntry
---@field entity LogicEntity
---@field gameobject any
---@field anim_end_event_id EventListenerID

---@class ClientWorld : World
---@field m_player_hint LogicEntity
---@field private _interp_delay number
---@field private _net_gameobjects table<NetID, ClientGameObject>
---@field private _idle_fx_pool FXEntry[]
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = World })

---@return ClientWorld
function _M.new()
    local self = World.new()
    ---@cast self ClientWorld
    self.m_player_hint = TL_Common.null_entity
    self._idle_fx_pool = {}
    self._net_gameobjects = {}
    self._interp_delay = 1.5 / TL_Common.GetContext():GetCommonConfig().m_server_fps
    return setmetatable(self, _M)
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
        entry.anim_end_event_id = ctx:GetEventSystem():AddAnimationEndEvent(
            function(id, event)
                if event:GetAnimationPlayerID() == anim_player:GetID() then
                    entry.gameobject.m_fx_component:Disable()
                    table.insert(self._idle_fx_pool, entry)
                end
            end)
    end
    entry.gameobject.m_fx_component:Play()
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

    local gameobject = ctx:GetScriptManager():Get(entity).m_gameobject
    local new_entry = { entity = entity, gameobject = gameobject, anim_end_event_id = TL_Common.null_event_listener_id }

    local anim_player = ctx:GetAnimationPlayerManager():Get(entity, 0)
    if anim_player then
        new_entry.anim_end_event_id = ctx:GetEventSystem():AddAnimationEndEvent(
            function(id, event)
                if event:GetAnimationPlayerID() == anim_player:GetID() then
                    gameobject.m_fx_component:Disable()
                    table.insert(self._idle_fx_pool, new_entry)
                end
            end)
    end
    gameobject.m_fx_component:Play()
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
    event_system:AddNetMsg_SpawnEvent(function(id, peer, spawn)
        self:onSpawn(spawn)
    end)
    event_system:AddNetMsg_ConnectEvent(function(id, peer, connect)
        self:onNetConnect(peer, connect)
    end)
    event_system:AddNetMsg_KillEvent(function(id, peer, msg)
        self:onPlayerKill(msg)
    end)
end

function _M:onNetConnect(peer, connect)
    local ctx = TL_Client.GetContext()
    local request = TL_Proto.SpawnPlayerRequest()
    request:set_m_entity(0)
    request:set_m_did(TL_Schema.DID.CharacterDID1)

    local msg = TL_Proto.NetMsg()
    msg:set_m_spawn_player_request(request)

    local host = ctx:GetNetHost()
    if host then
        host:Send(ctx:GetNetPeer(), msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
        ctx:Log("client sent SpawnPlayerRequest")
    end
end

---@param kill ProtoKill
function _M:onPlayerKill(kill)
    local net_id = kill:m_net_id()
    local gameobject = self._net_gameobjects[net_id]
    if gameobject == nil then
        return
    end

    gameobject:Destroy()
    self._net_gameobjects[net_id] = nil
end

--- Create any object the server tells us to spawn (players, items, fx, ...).
---@param spawn ProtoSpawn
function _M:onSpawn(spawn)
    local ctx = TL_Client.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return
    end

    -- lazy require: client.creation requires client.world at load time
    local ClientCreation = require("client.creation")
    local DID = require("common.did")

    local did = spawn:m_did()
    local net_id = spawn:m_net_id()
    local net_position = spawn:m_position()
    local position = TL_Common.Vec2(net_position:m_x(), net_position:m_y())

    local spawn_info = TL_Schema.ObjectSpawnDefinition()
    spawn_info.m_did = did
    spawn_info.m_spawn_on_layer = spawn:m_spawn_on_layer()

    local client_script = spawn:m_client_script()
    if client_script and client_script ~= "" then
        spawn_info.m_client_script = TL_Common.Path(client_script)
    end

    local entity = TL_Common.null_entity
    local gameobject = nil

    if DID.IsCharacterDID(did) then
        local hfsm_definition = nil
        if net_id ~= 0 and net_id == ctx:GetNetPeer():GetID() then
            -- The local player uses the level's player script + HFSM.
            local player_script = self.m_level_definition and self.m_level_definition.m_client_player_script
            if player_script and not player_script:empty() then
                spawn_info.m_client_script = player_script
            end
            local hfsm_path = self.m_level_definition and self.m_level_definition.m_player_related_definition.m_hfsm
            if hfsm_path and not hfsm_path:empty() then
                hfsm_definition = ctx:GetAssetsManager():GetScriptHFSMDefinitionManager():Load(hfsm_path)
            end
        end
        entity, gameobject = ClientCreation.CreateCharacter(ClientCreation, scene, spawn_info,
                                position, net_id, self.m_object_definitions, hfsm_definition)
    elseif DID.IsItemDID(did) then
        entity, gameobject = ClientCreation.CreateItem(ClientCreation, scene, spawn_info,
                                position, self.m_object_definitions)
    elseif DID.IsFXDID(did) then
        entity, gameobject = ClientCreation.CreateFX(ClientCreation, scene, spawn_info,
                                position, self.m_object_definitions)
    elseif DID.IsSkillDID(did) then
        entity, gameobject = ClientCreation.CreateSkill(ClientCreation, scene, spawn_info,
                                position, self.m_object_definitions)
    else
        ctx:Log("client spawn failed: unknown did ", did)
        return
    end

    if entity == TL_Common.null_entity then
        return
    end

    -- Attach to the layer the server spawned it on (falling back to the
    -- gameplay layer / scene root) so draw order matches.
    local parent_entity = nil
    if self.m_map_layers and spawn_info.m_spawn_on_layer ~= "" then
        parent_entity = self.m_map_layers[spawn_info.m_spawn_on_layer]
    end
    if parent_entity == nil or parent_entity == TL_Common.null_entity then
        parent_entity = self.m_land_entity
    end
    if parent_entity == nil or parent_entity == TL_Common.null_entity then
        parent_entity = scene:GetRootEntity()
    end
    local parent_relationship = ctx:GetRelationshipManager():Get(parent_entity)
    if parent_relationship then
        parent_relationship:AddChild(entity)
    end

    if net_id ~= 0 then
        if self._net_gameobjects[net_id] ~= nil then
            ctx:Log("client spawned duplicate! net_id: ", net_id, ", did ", did)
        end
        self._net_gameobjects[net_id] = gameobject
    end
    ctx:Log("client spawn object, net_id ", net_id, " did ", did)
end


return _M
