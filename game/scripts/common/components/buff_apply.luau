local Component = require("@common/components/component")

local _M = {}
_M.__index = _M
setmetatable(_M, {__index = Component})

type BuffApplyComponentData = {
    -- buffs this gameobject can apply to others (e.g. a skill's buffs)
    _buffs: { BuffDefinition },
}

export type BuffApplyComponent = typeof(_M) & BuffApplyComponentData & Component.Component

export type BuffApplyComponentDefinition = {
    m_buffs: { BuffDefinition },
}

function _M.new(gameobject, definition: BuffApplyComponentDefinition): BuffApplyComponent
    local self = Component.new(gameobject) :: BuffApplyComponent
    self._buffs = definition.m_buffs or {}
    return setmetatable(self, _M)
end

-- Apply all owned buffs onto the target gameobject's BuffReceiveComponent.
function _M.ApplyTo(self: BuffApplyComponent, target_gameobject)
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
