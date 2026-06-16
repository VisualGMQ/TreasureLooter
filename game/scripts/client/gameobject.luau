local ClientMoveComponent = require("@client/components/move")
local ClientHpComponent = require("@client/components/hp")
local ClientAttackComponent = require("@client/components/attack")
local ClientWeaponComponent = require("@client/components/weapon")
local GameObject = require("@common/gameobject")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = GameObject })

type ClientGameObjectData = {
    m_move_component:ClientMoveComponent.ClientMoveComponent,
    m_hp_component:ClientHpComponent.ClientHpComponent,
    m_attack_component:ClientAttackComponent.ClientAttackComponent,
    m_weapon_component:ClientWeaponComponent.ClientWeaponComponent,
}
export type ClientGameObject = typeof(_M) & ClientGameObjectData & GameObject.GameObject

export type ClientGameObjectDefinition = {
    m_move_component_definition:ClientMoveComponent.ClientMoveComponentDefinition,
    m_hp_component_definition:ClientHpComponent.ClientHpComponentDefinition,
    m_attack_component_definition:ClientAttackComponent.ClientAttackComponentDefinition,
    m_weapon_component_definition:ClientWeaponComponent.ClientWeaponComponentDefinition,
}

function _M.new(entity:Entity, definition:ClientGameObjectDefinition):ClientGameObject
    local self = GameObject.new(entity) :: ClientGameObject
    if definition.m_move_component_definition then
        self.m_move_component = ClientMoveComponent.new(self, definition.m_move_component_definition)
    end
    if definition.m_hp_component_definition then
        self.m_hp_component = ClientHpComponent.new(self, definition.m_hp_component_definition)
    end
    if definition.m_attack_component_definition then
        self.m_attack_component = ClientAttackComponent.new(self, definition.m_attack_component_definition)
    end
    if definition.m_weapon_component_definition then
        self.m_weapon_component = ClientWeaponComponent.new(self, definition.m_weapon_component_definition)
    end
    return setmetatable(self, _M)
end

function _M.GetEntity(self:ClientGameObject):Entity
    return self._entity
end

function _M.OnUpdate(self:ClientGameObject, elapse_time:TimeType)
    self.m_move_component:Update(elapse_time)
end

function _M.OnQuit(self:ClientGameObject)
    if self.m_attack_component then
        self.m_attack_component:OnQuit()
    end

    if self.m_move_component then
        self.m_move_component:OnQuit()
    end

    if self.m_hp_component then
        self.m_hp_component:OnQuit()
    end

    if self.m_weapon_component then
        self.m_weapon_component:OnQuit()
    end
end

return _M