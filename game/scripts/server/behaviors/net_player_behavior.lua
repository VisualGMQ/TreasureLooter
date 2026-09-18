local ServerGameObjectBehavior = require("server.gameobject_behavior")

---@class ServerNetPlayerBehavior : ServerGameObjectBehavior
---@field private _move_last_seq number
---@field private _broadcast_accum TimeType
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
    return self
end

function _M:OnInit()
    local ctx = TL_Common.GetContext()
    ctx:GetEventSystem():AddNetMsg_ClientActionEvent(
        function(id, peer, payload)
            self:onClientActionEvent(peer, payload)
        end)
end

---@private
---@param peer UDPPeer
---@param payload ProtoClientAction
function _M:onClientActionEvent(peer, payload)
    local ctx = TL_Server.GetContext()
    local go = self:GetGameObject()
    if not go or not go.m_move_component then
        return
    end

    -- The ClientAction event is delivered to every listener, so ignore actions
    -- that were not sent by the peer owning this player (net_id == the peer id
    -- stored when the player was spawned).
    if go:GetNetID() ~= peer:GetID() then
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
    local go = self:GetGameObject()
    if not go or not go.m_move_component then
        return
    end

    -- Broadcast at a fixed rate (independent of the input rate) so remote
    -- clients receive evenly spaced snapshots and can interpolate smoothly.
    local interval = 1.0 / TL_Common.GetContext():GetCommonConfig().m_server_fps
    self._broadcast_accum = self._broadcast_accum + elapse_time
    if self._broadcast_accum < interval then
        return
    end
    self._broadcast_accum = 0

    local position = go.m_transform:GetGlobalPosition()

    local net_position = TL_Proto.NetVec2()
    net_position:set_m_x(position.x)
    net_position:set_m_y(position.y)

    local move = TL_Proto.Move()
    move:set_m_net_id(go:GetNetID())
    move:set_m_entity(0)
    move:set_m_seq(self._move_last_seq)
    move:set_m_timestamp(TL_Server.GetContext():GetTime():GetCurrentTime())
    move:set_m_target(net_position)

    local net_msg = TL_Proto.NetMsg()
    net_msg:set_m_move(move)

    local host = TL_Common.GetContext():GetNetHost()
    if host then
        host:Broadcast(net_msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
    end
end

return _M
