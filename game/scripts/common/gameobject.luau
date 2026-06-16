local HpComponent = require("@common/components/hp")
local AttackComponent = require("@common/components/attack")
local _M = {}
_M.__index = _M

type GameObjectData = {
    m_transform:Transform,
    m_hp_component:HpComponent.HpComponent,
    m_attack_component:AttackComponent.AttackComponent,

    -- private fields
    _entity:Entity,
}
export type GameObject = typeof(_M) & GameObjectData

function _M.new(entity:Entity):GameObject
    local self = {}::GameObjectData
    local ctx = TL_Common.GetContext()
    self._entity = entity
    self.m_transform = ctx:GetTransformManager():Get(entity)
    return setmetatable(self, _M)
end

function _M.GetEntity(self:GameObject):Entity
    return self._entity
end

return _M