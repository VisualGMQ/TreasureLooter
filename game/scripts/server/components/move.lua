local MoveComponent = require("common.components.move")

---@class ServerMoveComponentDefinition
---@field m_cct CharacterController|nil
---@field m_speed number

---@class ServerMoveComponent : MoveComponent
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = MoveComponent })

---@param gameobject any
---@param definition ServerMoveComponentDefinition
---@return ServerMoveComponent
function _M.new(gameobject, definition)
    local self = MoveComponent.new(gameobject, definition.m_cct, definition.m_speed)
    ---@cast self ServerMoveComponent
    return setmetatable(self, _M)
end

return _M
