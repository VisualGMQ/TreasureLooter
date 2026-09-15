local Component = require("common.components.component")

---@class ItemComponentDefinition
---@field m_behavior ItemBehaviorFlags

---@class ItemComponent : Component
---@field _behavior ItemBehaviorFlags
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Component })

---@param gameobject any
---@param definition ItemComponentDefinition
---@return ItemComponent
function _M.new(gameobject, definition)
    local self = Component.new(gameobject)
    ---@cast self ItemComponent
    self._behavior = definition.m_behavior
    return setmetatable(self, _M)
end

---@return ItemBehaviorFlags
function _M:GetBehavior()
    return self._behavior
end

return _M
