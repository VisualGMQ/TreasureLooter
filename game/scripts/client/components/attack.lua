local AttackComponent = require("common.components.attack")

---@class ClientAttackComponent : AttackComponent
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = AttackComponent })

---@class ClientAttackComponentDefinition : AttackComponentDefinition

---@param gameobject any
---@param definition ClientAttackComponentDefinition
---@return ClientAttackComponent
function _M.new(gameobject, definition)
    local self = AttackComponent.new(gameobject, definition)
    ---@cast self ClientAttackComponent
    return setmetatable(self, _M)
end

return _M
