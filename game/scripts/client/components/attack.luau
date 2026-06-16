local AttackComponent = require("@common/components/attack")

local _M = {}
_M.__index = _M
setmetatable(_M, AttackComponent)

type ClientAttackComponentData = { }

export type ClientAttackComponent = typeof(_M) & ClientAttackComponentData & AttackComponent.AttackComponent

export type ClientAttackComponentDefinition = { } & AttackComponent.AttackComponentDefinition

function _M.new(gameobject, definition): ClientAttackComponent
    local self = AttackComponent.new(gameobject, definition)::ClientAttackComponent
    return setmetatable(self, _M)
end


return _M