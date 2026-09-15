local Component = require("common.components.component")
local World = require("common.world")

---@class BuffReceiveComponent : Component
---@field _buffs any[]
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Component })

---@param gameobject any
---@return BuffReceiveComponent
function _M.new(gameobject)
    local self = Component.new(gameobject)
    ---@cast self BuffReceiveComponent
    self._buffs = {}
    return setmetatable(self, _M)
end

--- Instantiate a buff from its definition (bound to this gameobject) and track it.
--- Skips application while the target is invincible. If a buff of the same type
--- is already active, it is refreshed to the new definition instead of stacking.
---@param buff_definition BuffDefinition
function _M:ApplyBuff(buff_definition)
    local go = self:GetGameObject()

    if go.m_hp_component and go.m_hp_component:IsInvincible() then
        return
    end

    for _, buff in ipairs(self._buffs) do
        if buff:GetType() == buff_definition.m_type then
            buff:Reset(buff_definition)
            return
        end
    end

    local appliers = World.GetInst().m_buff_appliers
    if not appliers then
        return
    end

    local buff = appliers:CreateBuff(buff_definition, go)
    if buff then
        table.insert(self._buffs, buff)
    end
end

--- True while a Freeze buff is active. Character behaviors query this to lock
--- the character (no movement / attacks) while frozen.
---@return boolean
function _M:IsFrozen()
    for _, buff in ipairs(self._buffs) do
        if buff:GetType() == TL_Schema.BuffType.Freeze then
            return true
        end
    end
    return false
end

---@param elapse_time TimeType
function _M:Update(elapse_time)
    -- iterate backwards so finished buffs can be removed in-place
    for i = #self._buffs, 1, -1 do
        local buff = self._buffs[i]
        buff:Update(elapse_time)
        if buff:IsFinished() then
            if buff.OnRemove then
                buff:OnRemove()
            end
            table.remove(self._buffs, i)
        end
    end
end

return _M
