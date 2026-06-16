local ClientGameObject = require("@client/gameobject")
local ClientAttackComponent = require("@client/components/attack")
local ClientMoveComponent = require("@client/components/move")
local ClientWeaponComponent = require("@client/components/weapon")
local Creation = require("@common/creation")
local _M = {}
_M.__index = _M
setmetatable(_M, {__index = Creation})

export type ClientCreation = typeof(_M)

function _M.ConvertCharacterDefinitionToGODefinition(entity:Entity, config:CharacterDefinitionHandle, weapon_entity:Entity):ClientGameObject.ClientGameObjectDefinition
    local definition = {}::ClientGameObject.ClientGameObjectDefinition
    local ctx = TL_Client.GetContext()
    local move_definition = {} ::ClientMoveComponent.ClientMoveComponentDefinition
    move_definition = {
        m_cct = ctx:GetCCTManager():Get(entity),
        m_speed = config.m_speed,
        m_move_down_anim = config.m_move_down_animation,
        m_move_up_anim = config.m_move_up_animation,
        m_move_left_anim = config.m_move_left_animation,
        m_move_right_anim = config.m_move_right_animation,
    }
    definition.m_move_component_definition = move_definition

    if weapon_entity ~= TL_Common.null_entity then
        local weapon_definition = {}::ClientWeaponComponent.ClientWeaponComponentDefinition
        weapon_definition.m_weapon_entity = weapon_entity
        definition.m_weapon_component_definition = weapon_definition
    end
    return definition
end

function _M.ConvertItemDefinitionToGODefinition(entity:Entity, config:ItemDefinitionHandle):ClientGameObject.ClientGameObjectDefinition
    local definition = {}::ClientGameObject.ClientGameObjectDefinition
    local ctx = TL_Client.GetContext()
    local attack_definition = {}::ClientAttackComponent.ClientAttackComponentDefinition

    local name_mgr = ctx:GetEntityNameManager()
    local weapon_entity = name_mgr:FindChildByName(entity, "weapon")

    if weapon_entity == TL_Common.null_entity then
        ctx:Log("create item failed: no weapon entity")
        return definition
    end

    if config.m_attackable then
        attack_definition.m_anim_player = ctx:GetAnimationPlayerManager():Get(weapon_entity)
        attack_definition.m_attack_anim = config.m_attackable.m_attack_animation
        attack_definition.m_cooldown = config.m_attackable.m_cooldown
        attack_definition.m_damage = config.m_attackable.m_damage
        attack_definition.m_hit_area = ctx:GetTriggerComponentManager():Get(weapon_entity)
        attack_definition.m_transform = ctx:GetTransformManager():Get(entity)
        definition.m_attack_component_definition = attack_definition
    end
    return definition

end

return _M