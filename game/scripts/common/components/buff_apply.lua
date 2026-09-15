local Component = require("common.components.component")

---@class BuffApplyComponentDefinition
---@field m_buffs BuffDefinition[]

---@class BuffApplyComponent : Component
---@field _buffs BuffDefinition[]
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Component })

---@param gameobject any
---@param definition BuffApplyComponentDefinition
---@return BuffApplyComponent
function _M.new(gameobject, definition)
    local self = Component.new(gameobject)
    ---@cast self BuffApplyComponent
    self._buffs = definition.m_buffs or {}
    return setmetatable(self, _M)
end

--- Apply all owned buffs onto the target gameobject's BuffReceiveComponent.
---@param target_gameobject any
function _M:ApplyTo(target_gameobject)
    if not target_gameobject then
        return
    end

    local receiver = target_gameobject.m_buff_receive_component
    if not receiver then
        return
    end

    for _, buff_def in ipairs(self._buffs) do
        receiver:ApplyBuff(buff_def)
    end
end

return _M
