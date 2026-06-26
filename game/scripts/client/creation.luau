local ClientGameObject = require("@client/gameobject")
local ClientAttackComponent = require("@client/components/attack")
local ClientMoveComponent = require("@client/components/move")
local ClientWeaponComponent = require("@client/components/weapon")
local ClientHpComponent = require("@client/components/hp")
local Creation = require("@common/creation")
local _M = {}
_M.__index = _M
setmetatable(_M, {__index = Creation})

export type ClientCreation = typeof(_M)

function _M.ConvertCharacterDefinitionToGODefinition(entity:Entity, config:CharacterDefinitionHandle, weapon_entity:Entity):ClientGameObject.ClientGameObjectDefinition
    local definition = {}::ClientGameObject.ClientGameObjectDefinition
    local ctx = TL_Client.GetContext()
    local multi_anim_players = ctx:GetMultiAnimationPlayerManager():Get(entity)

    local move_definition = {}::ClientMoveComponent.ClientMoveComponentDefinition
    move_definition.m_cct = ctx:GetCCTManager():Get(entity)
    move_definition.m_speed = config.m_speed
    move_definition.m_move_down_anim = config.m_move_down_animation
    move_definition.m_move_up_anim = config.m_move_up_animation
    move_definition.m_move_left_anim = config.m_move_left_animation
    move_definition.m_move_right_anim = config.m_move_right_animation
    move_definition.m_anim = multi_anim_players:GetAnimation(0)
    definition.m_move_component_definition = move_definition

    local hp_definition = {}::ClientHpComponent.ClientHpComponentDefinition
    hp_definition.m_anim_player = multi_anim_players:GetAnimation(1)
    hp_definition.m_hp = config.m_hp
    hp_definition.m_hurt_anim = config.m_hurt_animation
    hp_definition.m_dead_anim = config.m_dead_animation
    hp_definition.m_invincible_time = config.m_invincible_time
    definition.m_hp_component_definition = hp_definition

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
        attack_definition.m_anim_player = ctx:GetMultiAnimationPlayerManager():Get(weapon_entity):GetAnimation(0)
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