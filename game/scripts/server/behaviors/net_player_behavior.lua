local ServerGameObjectBehavior = require("server.gameobject_behavior")

---@class ServerNetPlayerBehavior : ServerGameObjectBehavior
---@field _has_input boolean  has received input from client
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ServerGameObjectBehavior })

---@param entity Entity
---@return ServerNetPlayerBehavior
function _M.new(entity)
    local self = setmetatable(ServerGameObjectBehavior.new(entity), _M)
    ---@cast self ServerNetPlayerBehavior
    self._has_input = false
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

    local action = payload:m_action()
    if action == TL_Schema.ClientActionType.ClientActionType_Move then
        if payload:has_m_move_disp() then
            local disp = payload:m_move_disp()
            go.m_move_component:AddMoveDisp(TL_Common.Vec2(disp:m_x(), disp:m_y()))
            self._has_input = true
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

    if not self._has_input then
        return
    end

    self._has_input = false
    go.m_move_component:Update()

    local position = go.m_transform:GetGlobalPosition()

    local net_position = TL_Proto.NetVec2()
    net_position:set_m_x(position.x)
    net_position:set_m_y(position.y)

    local move = TL_Proto.Move()
    move:set_m_entity(0)
    move:set_m_seq(0)
    move:set_m_target(net_position)

    local net_msg = TL_Proto.NetMsg()
    net_msg:set_m_move(move)

    local host = TL_Common.GetContext():GetNetHost()
    if host then
        host:Broadcast(net_msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
    end
end

return _M
