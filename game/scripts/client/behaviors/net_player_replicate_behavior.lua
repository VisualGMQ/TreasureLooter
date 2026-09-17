local ClientGameObjectBehavior = require("client.gameobject_behavior")

---@class ClientNetPlayerReplicateBehavior : ClientGameObjectBehavior
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

---@param entity LogicEntity
---@return ClientNetPlayerReplicateBehavior
function _M.new(entity)
    local self = setmetatable(ClientGameObjectBehavior.new(entity), _M)
    ---@cast self ClientNetPlayerReplicateBehavior
    return self
end

function _M:OnInit()
    local ctx = TL_Client.GetContext()
    ctx:GetEventSystem():AddNetMsg_MoveEvent(function(id, peer, move)
        self:onMoveNetEvent(move)
    end)
end

---@param move ProtoMove
---@private
function _M:onMoveNetEvent(move)
    local go = self:GetGameObject()
    if not go then
        return
    end

    -- the server broadcasts every player's move; only apply our own entity's
    if move:m_net_id() ~= go:GetNetID() then
        return
    end

    local net_target = move:m_target()
    local target = TL_Common.Vec2(net_target:m_x(), net_target:m_y())

    local move_component = go.m_move_component
    if move_component then
        move_component:Teleport(target)
    else
        go.m_transform.m_position = target
    end
end

return _M
