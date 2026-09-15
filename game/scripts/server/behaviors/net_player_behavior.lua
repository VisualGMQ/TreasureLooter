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
    ctx:GetEventSystem():AddNetMsg_ClientActionEvent(function(id, peer, payload)
        local go = self:GetGameObject()
        if not go or not go.m_move_component then
            return
        end

        if payload:has_m_move_dir() then
            local dir = payload:m_move_dir()
            go.m_move_component:SetDir(TL_Common.Vec2(dir:m_x(), dir:m_y()))
            self._has_input = true
        else
            ctx:Log("server received attack from entity ", self:GetEntity())
        end
    end)
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
    go.m_move_component:Update(elapse_time)

    local position = go.m_transform:GetGlobalPosition()

    local net_position = TL_Proto.NetVec2()
    net_position:set_m_x(position.x)
    net_position:set_m_y(position.y)

    local move = TL_Proto.Move()
    move:set_m_entity(0)
    move:set_m_seq(0)
    move:set_m_position(net_position)

    local net_msg = TL_Proto.NetMsg()
    net_msg:set_m_move(move)

    local host = TL_Common.GetContext():GetNetHost()
    if host then
        host:Broadcast(net_msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
    end
end

return _M
