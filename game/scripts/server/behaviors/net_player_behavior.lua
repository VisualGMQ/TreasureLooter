local ServerGameObjectBehavior = require("server.gameobject_behavior")
local ServerWorld = require("server.world")

---@class ServerNetPlayerBehavior : ServerGameObjectBehavior
---@field private _move_last_seq number
---@field private _broadcast_accum TimeType
--- The id of the `ClientAction` listener registered in `OnInit`. Removed in
--- `OnQuit`: the listener is owned by the global event system, not by the
--- entity, so it would otherwise keep firing (and leaking) after the player
--- entity was removed.
---@field private _action_listener EventListenerID|nil
--- Whether the behavior is still attached to a live round entity. Set to false
--- by `OnQuit` so late callbacks cannot use the removed player.
---@field private _active boolean
---@field  m_mass number
---@field private _max_speed number
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ServerGameObjectBehavior })

---@param entity LogicEntity
---@return ServerNetPlayerBehavior
function _M.new(entity)
    local self = setmetatable(ServerGameObjectBehavior.new(entity), _M)
    ---@cast self ServerNetPlayerBehavior
    self._move_last_seq = 0
    self._broadcast_accum = 0
    self._max_speed = 500
    self._action_listener = nil
    self._active = false
    self.m_mass = 10.0
    return self
end

function _M:OnInit()
    local ctx = TL_Common.GetContext()
    self._active = true
    self._action_listener = ctx:GetEventSystem():AddNetMsg_ClientActionEvent(
        function(id, peer, payload)
            self:onClientActionEvent(peer, payload)
        end)
end

function _M:OnQuit()
    self._active = false
    if self._action_listener then
        TL_Common.GetContext():GetEventSystem():Remove(self._action_listener)
        self._action_listener = nil
    end
    -- The game object owns the hp/move components (and the hp timer), so the
    -- base has to run too.
    ServerGameObjectBehavior.OnQuit(self)
end

---@private
---@param peer UDPPeer
---@param payload ProtoClientAction
function _M:onClientActionEvent(peer, payload)
    -- A removed player has no entity/round anymore: its listener may still be
    -- called during the frame the entity removal was queued, ignore it.
    if not self._active then
        return
    end

    local ctx = TL_Server.GetContext()
    local go = self:GetGameObject()
    if not go or not go.m_move_component then
        return
    end

    -- A dead player is not controlled anymore: ignore its inputs.
    if go.m_hp_component and go.m_hp_component:IsDead() then
        return
    end

    -- The ClientAction event is delivered to every listener, so ignore actions
    -- that were not sent by the peer owning this player (net_id == the peer id
    -- stored when the player was spawned).
    if go:GetNetID() ~= peer:GetID() then
        return
    end

    -- Players are frozen during the pre-game countdown: the server is the
    -- authority and drops every input until the countdown is over.
    local world = ServerWorld.GetInst()
    ---@cast world ServerWorld
    if not world:CanAcceptPlayerInput() then
        return
    end

    local action = payload:m_action()
    if action == TL_Schema.ClientActionType.ClientActionType_Move then
        if payload:has_m_move_disp() then
            local disp = payload:m_move_disp()
            go.m_move_component:SetMoveDisp(TL_Common.Vec2(disp:m_x(), disp:m_y()))
            go.m_move_component:Update()
            self._move_last_seq = payload:m_seq()
        else
            ctx:Log("don't has move displacement field: ", self:GetEntity())
        end
    elseif action == TL_Schema.ClientActionType.ClientActionType_Attack then
        ctx:Log("client sent attack action: ", self:GetEntity())
    end
end

---@param elapse_time TimeType
function _M:OnUpdate(elapse_time)
    if not self._active then
        return
    end

    local go = self:GetGameObject()
    if not go or not go.m_move_component then
        return
    end

    -- A dead player stops moving and is not replicated anymore.
    if go.m_hp_component and go.m_hp_component:IsDead() then
        return
    end

    -- Nothing moves nor is replicated before the countdown ends, so the
    -- clients cannot drift during the pre-game freeze.
    local world = ServerWorld.GetInst()
    ---@cast world ServerWorld
    if not world:CanAcceptPlayerInput() then
        return
    end

    -- Move by the server's own velocity (collisions) before broadcasting, so
    -- the push away is part of the authoritative position.
    go.m_move_component:Update(elapse_time)

    -- Broadcast at a fixed rate (independent of the input rate) so remote
    -- clients receive evenly spaced snapshots and can interpolate smoothly.
    local interval = 1.0 / TL_Common.GetContext():GetCommonConfig().m_server_fps
    self._broadcast_accum = self._broadcast_accum + elapse_time
    if self._broadcast_accum < interval then
        return
    end
    self._broadcast_accum = 0

    local position = go.m_transform:GetGlobalPosition()

    -- The velocity is the part of the movement the client cannot predict (a
    -- collision push), so it is sent too and applied by the clients.
    local velocity = go.m_move_component:GetVelocity()
    local net_velocity = TL_Proto.NetVec2()
    net_velocity:set_m_x(velocity.x)
    net_velocity:set_m_y(velocity.y)

    local net_position = TL_Proto.NetVec2()
    net_position:set_m_x(position.x)
    net_position:set_m_y(position.y)

    local move = TL_Proto.Move()
    move:set_m_net_id(go:GetNetID())
    move:set_m_entity(0)
    move:set_m_seq(self._move_last_seq)
    move:set_m_timestamp(TL_Server.GetContext():GetTime():GetCurrentTime())
    move:set_m_target(net_position)
    move:set_m_velocity(net_velocity)

    local net_msg = TL_Proto.NetMsg()
    net_msg:set_m_move(move)

    -- Unicast to the round players: a plain `UDPHost:Broadcast` would also hit
    -- peers the server already disconnected (and that the net host has not
    -- dropped yet), which fails and logs an error.
    world:broadcastToPlayers(net_msg)
end

return _M
