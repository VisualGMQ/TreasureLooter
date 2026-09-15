local ItemComponent = require("common.components.item")

---@class ClientItemComponentDefinition : ItemComponentDefinition

---@class ClientItemComponent : ItemComponent
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ItemComponent })

---@param gameobject any
---@param definition ClientItemComponentDefinition
---@return ClientItemComponent
function _M.new(gameobject, definition)
    local self = ItemComponent.new(gameobject, definition)
    ---@cast self ClientItemComponent
    return setmetatable(self, _M)
end

return _M
