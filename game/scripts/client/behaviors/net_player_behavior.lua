local ClientGameObjectBehavior = require("client.gameobject_behavior")
local ClientWorld = require("client.world")

---@class ClientMoveReconcilation
---@field seq number
---@field disp Vec2

---@class ClientNetPlayerBehavior : ClientGameObjectBehavior
---@field private _move_packet_seq number
---@field private _last_timestamp TimeType
---@field private _move_listener EventListenerID?
---@field private _reconcilation_list ClientMoveReconcilation[]
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

---@param entity LogicEntity
---@return ClientNetPlayerBehavior
function _M.new(entity)
    local self = setmetatable(ClientGameObjectBehavior.new(entity), _M)
    ---@cast self ClientNetPlayerBehavior
    self._move_packet_seq = 1
    self._last_timestamp = 0
    self._reconcilation_list = {}
    return self
end

function _M:OnInit()
    local ctx = TL_Client.GetContext()
    self._move_listener = ctx:GetEventSystem():AddNetMsg_MoveEvent(
        function(id, peer, payload)
            self:onMoveNetEvent(peer, payload)
        end)
end

-- Net listeners outlive the entity/script they were registered from (the
-- engine only destroys the script), so a stale one would keep reconciling a
-- character whose CharacterController was already freed with the scene.
function _M:OnQuit()
    if not self._move_listener then
        return
    end

    TL_Client.GetContext():GetEventSystem():Remove(self._move_listener)
    self._move_listener = nil
end

---@param peer UDPPeer
---@param move ProtoMove
---@private
function _M:onMoveNetEvent(peer, move)
    local go = self:GetGameObject()
    if not go or not go.m_move_component then
        return
    end

    -- The listener can still fire in the frame the game scene is unloaded:
    -- never touch a character whose physics was already destroyed.
    if not TL_Client.GetContext():GetTransformManager():Has(go:GetEntity()) then
        return
    end

    -- the server broadcasts every player's move; only reconcile our own entity
    if move:m_net_id() ~= go:GetNetID() then
        return
    end

    -- The authoritative position can change without a newer input being
    -- acknowledged (a collision pushes a player that is not moving), so the
    -- freshness is checked on the broadcast timestamp; the input seq is only
    -- used to drop the inputs the server already applied.
    local timestamp = move:m_timestamp()
    if timestamp <= self._last_timestamp then
        return
    end
    self._last_timestamp = timestamp

    local seq = move:m_seq()
    -- physics transform: snap straight to the authoritative position, then
    -- replay the inputs the server has not acknowledged yet
    local net_target = move:m_target()
    local target = TL_Common.Vec2(net_target:m_x(), net_target:m_y())

    if move:has_m_velocity() then
        local net_velocity = move:m_velocity()
        go.m_move_component:SetServerVelocity(
            TL_Common.Vec2(net_velocity:m_x(), net_velocity:m_y()))
    end

    -- Snap to the authoritative position and replay the inputs the server has
    -- not applied yet, so the local prediction keeps leading by exactly those
    -- inputs. Both happen in the same frame, so the snap is never shown.
    go.m_move_component:Teleport(target)

    -- do reconciliation
    while #self._reconcilation_list > 0
        and self._reconcilation_list[1].seq <= seq do
        table.remove(self._reconcilation_list, 1)
    end

    for _, elem in ipairs(self._reconcilation_list) do
        go.m_move_component:SetMoveDisp(elem.disp)
        go.m_move_component:Update()
    end
end

---@param elapse_time TimeType
function _M:OnUpdate(elapse_time)
    local go = self:GetGameObject()
    if not go then
        return
    end

    local world = ClientWorld.FindInst()
    ---@cast world ClientWorld|nil
    local hp_component = go.m_hp_component
    if world and hp_component then
        world:SetLocalHp(hp_component:GetHp())
    end

    local move_component = go.m_move_component
    local game_over = world and world:IsGameOver()
    local control_locked = world and world:IsControlLocked()
    local dead = hp_component and hp_component:IsDead()
    if game_over or control_locked or dead then
        -- Dead, round over or waiting for the start countdown: stop operating
        -- (spectate). Do not read the input nor send any ClientAction, only
        -- fall back to the idle animation.
        if move_component then
            move_component:SetDir(TL_Common.Vec2.ZERO)
            move_component:Update(elapse_time)
        end
        return
    end

    local ctx = TL_Client.GetContext()
    local host = ctx:GetNetHost()
    local peer = ctx:GetNetPeer()
    -- The peer can be gone (the server dropped this client): never send to a
    -- disconnected peer, `enet_peer_send` would fail on every frame.
    if not host or not peer or not peer:IsValid() then
        return
    end

    local input_manager = ctx:GetInputManager()
    local action = TL_Proto.ClientAction()

    local action_type = nil

    local attack = input_manager:GetAction("Attack")
    if attack:IsPressed(0) then
        action_type = TL_Schema.ClientActionType.ClientActionType_Attack
    end

    if move_component then
        if action_type ~= nil then
            -- attacking: drop the velocity so the walk animation stops
            move_component:SetDir(TL_Common.Vec2.ZERO)
        else
            local axises = input_manager:MakeAxises("MoveX", "MoveY"):Value(0)
            if axises:LengthSquared() ~= 0 then
                move_component:SetDir(axises)
                move_component:Accelerate(axises, elapse_time)
                local disp = move_component:GetVelocity() *  elapse_time
                -- send move packet to net
                action_type = TL_Schema.ClientActionType.ClientActionType_Move
                local net_disp = TL_Proto.NetVec2()
                net_disp:set_m_x(disp.x)
                net_disp:set_m_y(disp.y)
                action:set_m_move_disp(net_disp)
                action:set_m_seq(self._move_packet_seq)

                -- client move first
                move_component:SetMoveDisp(disp)

                -- remember it so it can be replayed until the server acks it
                local reconciliation = {
                    seq = self._move_packet_seq,
                    disp = disp
                }
                table.insert(self._reconcilation_list, reconciliation)

                self._move_packet_seq = self._move_packet_seq + 1
            else
                -- released the keys: clearing the velocity is what lets the
                -- component fall back to the idle frame (otherwise the walk
                -- animation keeps playing forever)
                move_component:SetDir(TL_Common.Vec2.ZERO)
            end
        end

        -- must run every frame, moving or not: this is what plays the walk
        -- animation and stops it again when the velocity is zero
        move_component:Update(elapse_time)
    end

    if action_type ~= nil then
        action:set_m_action(action_type)
        local net_msg = TL_Proto.NetMsg()
        net_msg:set_m_client_action(action)
        host:Send(peer, net_msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
    end
end

-- Follow the character in the render phase so the camera and the sprite sample
-- the exact same (global) transform value; updating in OnUpdate would read a
-- stale global matrix from before RelationshipManager::Update and cause jitter.
function _M:OnRender()
    local go = self:GetGameObject()
    if not go or not go.m_transform then
        return
    end
    local ctx = TL_Client.GetContext()
    -- The entity can be destroyed by the death animation while this behavior
    -- is still alive for a frame: don't follow a dangling transform.
    if not ctx:GetTransformManager():Has(go:GetEntity()) then
        return
    end
    ctx:GetCamera():MoveTo(go.m_transform:GetGlobalPosition())
end

return _M
