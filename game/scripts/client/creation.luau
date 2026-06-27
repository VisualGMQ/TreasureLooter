local ClientGameObject = require("@client/gameobject")
local ClientAttackComponent = require("@client/components/attack")
local ClientMoveComponent = require("@client/components/move")
local ClientWeaponComponent = require("@client/components/weapon")
local ClientHpComponent = require("@client/components/hp")
local ClientItemComponent = require("@client/components/item")
local ClientInteractComponent = require("@client/components/interact")
local ClientFXComponent = require("@client/components/fx")
local ClientWorld = require("@client/world")
local Creation = require("@common/creation")
local _M = {}
_M.__index = _M
setmetatable(_M, {__index = Creation})

export type ClientCreation = typeof(_M)

function _M.ConvertCharacterDefinitionToGODefinition(entity:Entity, did:DID, config:CharacterDefinitionHandle, weapon_entity:Entity):ClientGameObject.ClientGameObjectDefinition
    local definition = {}::ClientGameObject.ClientGameObjectDefinition
    local ctx = TL_Client.GetContext()
    local multi_anim_players = ctx:GetMultiAnimationPlayerManager():Get(entity)

    definition.m_did = did

    local move_definition = {}::ClientMoveComponent.ClientMoveComponentDefinition
    move_definition.m_cct = ctx:GetCCTManager():Get(entity)
    move_definition.m_speed = config.m_move.m_speed
    move_definition.m_move_down_anim = config.m_move.m_move_down_animation
    move_definition.m_move_up_anim = config.m_move.m_move_up_animation
    move_definition.m_move_left_anim = config.m_move.m_move_left_animation
    move_definition.m_move_right_anim = config.m_move.m_move_right_animation
    move_definition.m_anim = multi_anim_players:GetAnimation(0)
    definition.m_move_component_definition = move_definition

    local hp_definition = {}::ClientHpComponent.ClientHpComponentDefinition
    hp_definition.m_anim_player = multi_anim_players:GetAnimation(1)
    hp_definition.m_hp = config.m_hp.m_hp
    hp_definition.m_hurt_anim = config.m_hp.m_hurt_animation
    hp_definition.m_dead_anim = config.m_hp.m_dead_animation
    hp_definition.m_invincible_time = config.m_hp.m_invincible_time
    definition.m_hp_component_definition = hp_definition

    if weapon_entity ~= TL_Common.null_entity then
        local weapon_definition = {}::ClientWeaponComponent.ClientWeaponComponentDefinition
        weapon_definition.m_weapon_entity = weapon_entity
        definition.m_weapon_component_definition = weapon_definition
    end

    if config.m_interact then
        local interact_definition = {}::ClientInteractComponent.ClientInteractComponentDefinition
        interact_definition.m_interact_area = ctx:GetTriggerComponentManager():Get(entity)
        interact_definition.m_detect_range = config.m_interact.m_detect_range
        interact_definition.m_can_interact_range = config.m_interact.m_can_interact_range
        definition.m_interact_component_definition = interact_definition
    end

    return definition
end

function _M.ConvertItemDefinitionToGODefinition(entity:Entity, did:DID, config:ItemDefinitionHandle):ClientGameObject.ClientGameObjectDefinition
    local definition = {}::ClientGameObject.ClientGameObjectDefinition

    definition.m_did = did

    local item_definition = {}::ClientItemComponent.ClientItemComponentDefinition
    item_definition.m_behavior = config.m_behavior
    definition.m_item_component_definition = item_definition

    return definition
end

-- the weapon controller hosts the attack & weapon components, driving the weapon item under it
function _M.ConvertWeaponControllerToGODefinition(controller_entity:Entity, weapon_item_entity:Entity, did:DID, config:ItemDefinitionHandle):ClientGameObject.ClientGameObjectDefinition
    local definition = {}::ClientGameObject.ClientGameObjectDefinition
    local ctx = TL_Client.GetContext()

    definition.m_did = did

    if config.m_attackable then
        local attack_definition = {}::ClientAttackComponent.ClientAttackComponentDefinition
        attack_definition.m_anim_player = ctx:GetMultiAnimationPlayerManager():Get(weapon_item_entity):GetAnimation(0)
        attack_definition.m_attack_anim = config.m_attackable.m_attack_animation
        attack_definition.m_cooldown = config.m_attackable.m_cooldown
        attack_definition.m_damage = config.m_attackable.m_damage
        attack_definition.m_hit_area = ctx:GetTriggerComponentManager():Get(weapon_item_entity)
        attack_definition.m_transform = ctx:GetTransformManager():Get(controller_entity)
        definition.m_attack_component_definition = attack_definition
    end

    local weapon_definition = {}::ClientWeaponComponent.ClientWeaponComponentDefinition
    weapon_definition.m_weapon_entity = weapon_item_entity
    definition.m_weapon_component_definition = weapon_definition

    return definition
end

function _M.ConvertFXDefinitionToGODefinition(entity:Entity, did:DID, config:FXDefinitionHandle):ClientGameObject.ClientGameObjectDefinition
    local definition = {}::ClientGameObject.ClientGameObjectDefinition
    local ctx = TL_Client.GetContext()

    definition.m_did = did

    local fx_definition = {}::ClientFXComponent.FXComponentDefinition
    local multi_anim_players = ctx:GetMultiAnimationPlayerManager():Get(entity)
    if multi_anim_players then
        fx_definition.m_anim_player = multi_anim_players:GetAnimation(0)
    end
    definition.m_fx_component_definition = fx_definition

    return definition
end

-- create the player interactable hint fx entity and store it in ClientWorld.m_player_hint
function _M.CreatePlayerHintFX(scene:Scene, did:DID, config:FXDefinitionHandle, client_script:Path):Entity
    local ctx = TL_Client.GetContext()

    local prefab = Creation.ConvertFXDefinitionToPrefab(config)
    prefab.m_client_script = client_script
    local entity = scene:Instantiate(prefab, nil)
    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    local root_relationship = ctx:GetRelationshipManager():Get(scene:GetRootEntity())
    if root_relationship then
        root_relationship:AddChild(entity)
    end

    local script = ctx:GetScriptManager():Get(entity)
    if script then
        local go_definition = _M.ConvertFXDefinitionToGODefinition(entity, did, config)
        script:initGameObject(go_definition)

        if script.m_gameobject and script.m_gameobject.m_fx_component then
            script.m_gameobject.m_fx_component:Disable()
        end
    end

    local world = ClientWorld.GetInst() :: ClientWorld.ClientWorld
    world.m_player_hint = entity

    return entity
end

return _M