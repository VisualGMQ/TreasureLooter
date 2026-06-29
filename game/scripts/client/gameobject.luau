local ClientMoveComponent = require("@client/components/move")
local ClientHpComponent = require("@client/components/hp")
local ClientAttackComponent = require("@client/components/attack")
local ClientWeaponComponent = require("@client/components/weapon")
local ClientItemComponent = require("@client/components/item")
local ClientInteractComponent = require("@client/components/interact")
local ClientFXComponent = require("@client/components/fx")
local ClientRaiseUpComponent = require("@client/components/raise_up")
local ClientSpawnObjectComponent = require("@client/components/spawn_object")
local ClientSpawnedComponent = require("@client/components/spawned")
local GameObject = require("@common/gameobject")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = GameObject })

type ClientGameObjectData = {
    m_move_component:ClientMoveComponent.ClientMoveComponent,
    m_hp_component:ClientHpComponent.ClientHpComponent,
    m_attack_component:ClientAttackComponent.ClientAttackComponent,
    m_weapon_component:ClientWeaponComponent.ClientWeaponComponent,
    m_item_component:ClientItemComponent.ClientItemComponent,
    m_interact_component:ClientInteractComponent.ClientInteractComponent,
    m_fx_component:ClientFXComponent.FXComponent,
    m_raise_up_component:ClientRaiseUpComponent.ClientRaiseUpComponent,
    m_spawn_object_component:ClientSpawnObjectComponent.ClientSpawnObjectComponent,
    m_spawned_component:ClientSpawnedComponent.ClientSpawnedComponent,
}
export type ClientGameObject = typeof(_M) & ClientGameObjectData & GameObject.GameObject

export type ClientGameObjectDefinition = {
    m_did:DID,
    m_move_component_definition:ClientMoveComponent.ClientMoveComponentDefinition,
    m_hp_component_definition:ClientHpComponent.ClientHpComponentDefinition,
    m_attack_component_definition:ClientAttackComponent.ClientAttackComponentDefinition,
    m_weapon_component_definition:ClientWeaponComponent.ClientWeaponComponentDefinition,
    m_item_component_definition:ClientItemComponent.ClientItemComponentDefinition,
    m_interact_component_definition:ClientInteractComponent.ClientInteractComponentDefinition,
    m_fx_component_definition:ClientFXComponent.FXComponentDefinition,
    m_spawn_object_component_definition:ClientSpawnObjectComponent.ClientSpawnObjectComponentDefinition,
}

function _M.new(entity:Entity, definition:ClientGameObjectDefinition):ClientGameObject
    local self = GameObject.new(entity, definition.m_did) :: ClientGameObject
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
    if definition.m_item_component_definition then
        self.m_item_component = ClientItemComponent.new(self, definition.m_item_component_definition)
    end
    if definition.m_interact_component_definition then
        self.m_interact_component = ClientInteractComponent.new(self, definition.m_interact_component_definition)
    end
    if definition.m_fx_component_definition then
        self.m_fx_component = ClientFXComponent.new(self, definition.m_fx_component_definition)
    end
    if definition.m_spawn_object_component_definition then
        self.m_spawn_object_component = ClientSpawnObjectComponent.new(self, definition.m_spawn_object_component_definition)
    end
    self.m_raise_up_component = ClientRaiseUpComponent.new(self)
    return setmetatable(self, _M)
end

function _M.GetEntity(self:ClientGameObject):Entity
    return self._entity
end

function _M.OnUpdate(self:ClientGameObject, elapse_time:TimeType)
    if self.m_spawned_component then
        self.m_spawned_component:Update(elapse_time)
    end
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

    if self.m_item_component then
        self.m_item_component:OnQuit()
    end

    if self.m_interact_component then
        self.m_interact_component:OnQuit()
    end

    if self.m_fx_component then
        self.m_fx_component:OnQuit()
    end

    if self.m_raise_up_component then
        self.m_raise_up_component:OnQuit()
    end

    if self.m_spawn_object_component then
        self.m_spawn_object_component:OnQuit()
    end

    if self.m_spawned_component then
        self.m_spawned_component:OnQuit()
    end
end

return _M