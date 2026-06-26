local Component = require("@common/components/component")

local _M = {}
_M.__index = _M
setmetatable(_M, Component)

type ItemComponentData = {
    _behavior:ItemBehaviorFlags,
}

export type ItemComponent = typeof(_M) & ItemComponentData & Component.Component

export type ItemComponentDefinition = {
    m_behavior:ItemBehaviorFlags,
}

function _M.new(gameobject, definition:ItemComponentDefinition):ItemComponent
    local self = Component.new(gameobject) :: ItemComponent
    self._behavior = definition.m_behavior
    return setmetatable(self, _M)
end

function _M.GetBehavior(self:ItemComponent):ItemBehaviorFlags
    return self._behavior
end

return _M
