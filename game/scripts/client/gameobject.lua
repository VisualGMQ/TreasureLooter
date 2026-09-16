local ClientMoveComponent = require("client.components.move")
local ClientHpComponent = require("client.components.hp")
local ClientAttackComponent = require("client.components.attack")
local ClientWeaponComponent = require("client.components.weapon")
local ClientItemComponent = require("client.components.item")
local ClientInteractComponent = require("client.components.interact")
local ClientFXComponent = require("client.components.fx")
local ClientRaiseUpComponent = require("client.components.raise_up")
local ClientSpawnObjectComponent = require("client.components.spawn_object")
local ClientSpawnedComponent = require("client.components.spawned")
local ClientConstructableComponent = require("client.components.constructable")
local ClientSkillComponent = require("client.components.skill")
local BuffReceiveComponent = require("common.components.buff_receive")
local BuffApplyComponent = require("common.components.buff_apply")
local GameObject = require("common.gameobject")

---@class ClientGameObjectData
---@field m_move_component ClientMoveComponent
---@field m_hp_component ClientHpComponent
---@field m_attack_component ClientAttackComponent
---@field m_weapon_component ClientWeaponComponent
---@field m_item_component ClientItemComponent
---@field m_interact_component ClientInteractComponent
---@field m_fx_component FXComponent
---@field m_raise_up_component ClientRaiseUpComponent
---@field m_spawn_object_component ClientSpawnObjectComponent
---@field m_spawned_component ClientSpawnedComponent
---@field m_constructable_component ClientConstructableComponent
---@field m_skill_components ClientSkillComponent[]
---@field m_buff_receive_component BuffReceiveComponent
---@field m_buff_apply_component BuffApplyComponent

---@class ClientGameObject : GameObject
---@field m_move_component ClientMoveComponent|nil
---@field m_hp_component ClientHpComponent|nil
---@field m_attack_component ClientAttackComponent|nil
---@field m_weapon_component ClientWeaponComponent|nil
---@field m_item_component ClientItemComponent|nil
---@field m_interact_component ClientInteractComponent|nil
---@field m_fx_component FXComponent|nil
---@field m_raise_up_component ClientRaiseUpComponent|nil
---@field m_spawn_object_component ClientSpawnObjectComponent|nil
---@field m_spawned_component ClientSpawnedComponent|nil
---@field m_constructable_component ClientConstructableComponent|nil
---@field m_skill_components ClientSkillComponent[]
---@field m_buff_receive_component BuffReceiveComponent
---@field m_buff_apply_component BuffApplyComponent|nil
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = GameObject })

---@class ClientGameObjectDefinition
---@field m_did DID
---@field m_move_component_definition ClientMoveComponentDefinition|nil
---@field m_hp_component_definition ClientHpComponentDefinition|nil
---@field m_attack_component_definition ClientAttackComponentDefinition|nil
---@field m_weapon_component_definition ClientWeaponComponentDefinition|nil
---@field m_item_component_definition ClientItemComponentDefinition|nil
---@field m_interact_component_definition ClientInteractComponentDefinition|nil
---@field m_fx_component_definition FXComponentDefinition|nil
---@field m_spawn_object_component_definition ClientSpawnObjectComponentDefinition|nil
---@field m_constructable_component_definition ClientConstructableComponentDefinition|nil
---@field m_skill_component_definitions ClientSkillComponentDefinition[]|nil
---@field m_buff_apply_component_definition BuffApplyComponentDefinition|nil

---@param entity LogicEntity
---@param definition ClientGameObjectDefinition
---@return ClientGameObject
function _M.new(entity, definition)
    local self = GameObject.new(entity, definition.m_did)
    ---@cast self ClientGameObject
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
    if definition.m_constructable_component_definition then
        self.m_constructable_component = ClientConstructableComponent.new(self, definition.m_constructable_component_definition)
    end
    self.m_skill_components = {}
    if definition.m_skill_component_definitions then
        for _, skill_def in ipairs(definition.m_skill_component_definitions) do
            table.insert(self.m_skill_components, ClientSkillComponent.new(self, skill_def))
        end
    end
    if definition.m_buff_apply_component_definition then
        self.m_buff_apply_component = BuffApplyComponent.new(self, definition.m_buff_apply_component_definition)
    end
    self.m_buff_receive_component = BuffReceiveComponent.new(self)
    self.m_raise_up_component = ClientRaiseUpComponent.new(self)
    return setmetatable(self, _M)
end

---@return LogicEntity
function _M:GetEntity()
    return self._entity
end

---@param elapse_time TimeType
function _M:OnUpdate(elapse_time)
    if self.m_spawned_component then
        self.m_spawned_component:Update(elapse_time)
    end
    for _, skill_component in ipairs(self.m_skill_components) do
        skill_component:Update(elapse_time)
    end
end

function _M:OnQuit()
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

    if self.m_constructable_component then
        self.m_constructable_component:OnQuit()
    end

    if self.m_logic_component then
        self.m_logic_component:OnQuit()
    end

    for _, skill_component in ipairs(self.m_skill_components) do
        skill_component:OnQuit()
    end
end

return _M
