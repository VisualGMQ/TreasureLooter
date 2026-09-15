local Component = require("common.components.component")

---@class WeaponComponent : Component
---@field _weapon_script any
local _M = {}
_M.__index = {}
setmetatable(_M, { __index = Component })

---@param gameobject any
---@return WeaponComponent
function _M.new(gameobject)
    local self = Component.new(gameobject)
    ---@cast self WeaponComponent
    return setmetatable(self, _M)
end

---@return any
function _M:GetScript()
    return self._weapon_script
end

return _M
