local ClientWorld = require("client.world")
local Creation = require("common.creation")

---@class ClientCreation : Creation
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Creation })

local k_default_script = TL_Common.Path("scripts/client/gameobject_behavior.lua")

---@param entity LogicEntity
---@param did DID
---@param config CharacterDefinitionHandle
---@param weapon_entity LogicEntity
---@return ClientGameObjectDefinition
function _M.ConvertCharacterDefinitionToGODefinition(entity, did, config, weapon_entity)
    local definition = {}
    local ctx = TL_Client.GetContext()
    local anim_mgr = ctx:GetAnimationPlayerManager()

    definition.m_did = did

    local move_definition = {}
    move_definition.m_cct = ctx:GetCCTManager():Get(entity)
    move_definition.m_speed = config.m_move.m_speed
    move_definition.m_move_down_anim = config.m_move.m_move_down_animation
    move_definition.m_move_up_anim = config.m_move.m_move_up_animation
    move_definition.m_move_left_anim = config.m_move.m_move_left_animation
    move_definition.m_move_right_anim = config.m_move.m_move_right_animation
    move_definition.m_anim = anim_mgr:Get(entity, 0)
    definition.m_move_component_definition = move_definition

    local hp_definition = {}
    hp_definition.m_anim_player = anim_mgr:Get(entity, 1)
    hp_definition.m_hp = config.m_hp.m_hp
    hp_definition.m_hurt_anim = config.m_hp.m_hurt_animation
    hp_definition.m_dead_anim = config.m_hp.m_dead_animation
    hp_definition.m_invincible_time = config.m_hp.m_invincible_time
    definition.m_hp_component_definition = hp_definition

    if weapon_entity ~= TL_Common.null_entity then
        local weapon_definition = {}
        weapon_definition.m_weapon_entity = weapon_entity
        definition.m_weapon_component_definition = weapon_definition
    end

    if config.m_interact then
        local interact_definition = {}
        interact_definition.m_interact_area = ctx:GetTriggerComponentManager():Get(entity)
        interact_definition.m_detect_range = config.m_interact.m_detect_range
        interact_definition.m_can_interact_range = config.m_interact.m_can_interact_range
        definition.m_interact_component_definition = interact_definition
    end

    return definition
end

---@param entity LogicEntity
---@param did DID
---@param config ItemDefinitionHandle
---@return ClientGameObjectDefinition
function _M.ConvertItemDefinitionToGODefinition(entity, did, config)
    local definition = {}

    definition.m_did = did

    local item_definition = {}
    item_definition.m_behavior = config.m_behavior
    definition.m_item_component_definition = item_definition

    if config.m_spawn_object then
        local spawn_object_definition = {}
        spawn_object_definition.m_object_did = config.m_spawn_object.m_object_did
        spawn_object_definition.m_count = config.m_spawn_object.m_count
        spawn_object_definition.m_spawn_animations = config.m_spawn_object.m_spawn_animations
        definition.m_spawn_object_component_definition = spawn_object_definition
    end

    if config.m_constructable then
        local constructable_definition = {}
        constructable_definition.m_phases = config.m_constructable.m_phases
        constructable_definition.m_material_accept_range = config.m_constructable.m_material_accept_range
        constructable_definition.m_fx_on_accept_material = config.m_constructable.m_fx_on_accept_material
        definition.m_constructable_component_definition = constructable_definition
    end

    return definition
end

---@param entity LogicEntity
---@param skill_definition SkillDefinitionHandle
---@return ClientGameObjectDefinition
function _M.ConvertSkillDefinitionToGODefinition(entity, skill_definition)
    local definition = {}
    local ctx = TL_Client.GetContext()

    definition.m_did = TL_Schema.DID.Invalid

    local skill_def = {}
    skill_def.m_skill_definition = skill_definition
    local anim_player = ctx:GetAnimationPlayerManager():Get(entity, 0)
    if anim_player then
        skill_def.m_anim_player = anim_player
    end
    definition.m_skill_component_definitions = { skill_def }

    if skill_definition.m_buffs and #skill_definition.m_buffs > 0 then
        definition.m_buff_apply_component_definition = { m_buffs = skill_definition.m_buffs }
    end

    return definition
end

---@param self ClientCreation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param object_definitions ObjectDefinitionTable
---@return LogicEntity, GameObject
function _M:CreateSkill(scene, spawn_info, position, object_definitions)
    local ctx = TL_Client.GetContext()

    local skill_definition = object_definitions:GetSkill(spawn_info.m_did)
    local client_script = spawn_info.m_client_script
    if client_script:empty() then
        client_script = skill_definition.m_script
    end
    if client_script:empty() then
        client_script = k_default_script
    end

    local prefab = Creation.ConvertSkillDefinitionToPrefab(skill_definition)
    prefab.m_client_script = client_script

    local transform = TL_Common.Transform()
    transform.m_position = position

    local entity = scene:Instantiate(prefab, transform)
    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    local script = ctx:GetScriptManager():Get(entity)
    if script then
        local go_definition = _M.ConvertSkillDefinitionToGODefinition(entity, skill_definition)
        script:initGameObject(go_definition)
    end

    return entity, script and script.m_gameobject
end

--- The weapon controller hosts the attack & weapon components, driving the weapon item under it.
---@param controller_entity LogicEntity
---@param weapon_item_entity LogicEntity
---@param did DID
---@param config ItemDefinitionHandle
---@return ClientGameObjectDefinition
function _M.ConvertWeaponControllerToGODefinition(controller_entity, weapon_item_entity, did, config)
    local definition = {}
    local ctx = TL_Client.GetContext()

    definition.m_did = did

    if config.m_attackable then
        local attack_definition = {}
        attack_definition.m_anim_player = ctx:GetAnimationPlayerManager():Get(weapon_item_entity, 0)
        attack_definition.m_attack_anim = config.m_attackable.m_attack_animation
        attack_definition.m_cooldown = config.m_attackable.m_cooldown
        attack_definition.m_damage = config.m_attackable.m_damage
        attack_definition.m_hit_area = ctx:GetTriggerComponentManager():Get(weapon_item_entity)
        attack_definition.m_transform = ctx:GetTransformManager():Get(controller_entity)
        definition.m_attack_component_definition = attack_definition
    end

    local weapon_definition = {}
    weapon_definition.m_weapon_entity = weapon_item_entity
    definition.m_weapon_component_definition = weapon_definition

    return definition
end

---@param entity LogicEntity
---@param did DID
---@param config FXDefinitionHandle
---@return ClientGameObjectDefinition
function _M.ConvertFXDefinitionToGODefinition(entity, did, config)
    local definition = {}
    local ctx = TL_Client.GetContext()

    definition.m_did = did

    local fx_definition = {}
    local anim_player = ctx:GetAnimationPlayerManager():Get(entity, 0)
    if anim_player then
        fx_definition.m_anim_player = anim_player
    end
    definition.m_fx_component_definition = fx_definition

    return definition
end

---@param self ClientCreation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param object_definitions ObjectDefinitionTable
---@return LogicEntity, GameObject
function _M:CreateFX(scene, spawn_info, position, object_definitions)
    local ctx = TL_Client.GetContext()

    local fx_definition = object_definitions:GetFX(spawn_info.m_did)

    local prefab = Creation.ConvertFXDefinitionToPrefab(fx_definition)

    if spawn_info.m_client_script:empty() then
        prefab.m_client_script = k_default_script
    else
        prefab.m_client_script = spawn_info.m_client_script
    end

    local transform = TL_Common.Transform()
    transform.m_position = position

    local entity = scene:Instantiate(prefab, transform)
    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    local script = ctx:GetScriptManager():Get(entity)
    if script then
        local go_definition = _M.ConvertFXDefinitionToGODefinition(entity, TL_Schema.DID.Invalid, fx_definition)
        script:initGameObject(go_definition)
    end
    return entity, script and script.m_gameobject
end

--- Create the player interactable hint fx entity and store it in ClientWorld.m_player_hint.
---@param scene Scene
---@param did DID
---@param config FXDefinitionHandle
---@param client_script Path
---@return LogicEntity
function _M.CreatePlayerHintFX(scene, did, config, client_script)
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

    local world = ClientWorld.GetInst()
    ---@cast world ClientWorld
    world.m_player_hint = entity

    return entity
end

---@param self ClientCreation
---@param scene Scene
---@param prefab PrefabHandle
---@param transform Transform
---@return LogicEntity
function _M:CreatePrefab(scene, prefab, transform)
    return scene:Instantiate(prefab, transform)
end

---@param self ClientCreation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param object_definitions ObjectDefinitionTable
---@return LogicEntity, GameObject
function _M:CreateItem(scene, spawn_info, position, object_definitions)
    local ctx = TL_Client.GetContext()

    local transform = TL_Common.Transform()
    transform.m_position = position

    local definition = object_definitions:GetItem(spawn_info.m_did)
    local prefab = _M.ConvertItemDefinitionToPrefab(definition)

    if spawn_info.m_client_script:empty() then
        prefab.m_client_script = k_default_script
    else
        prefab.m_client_script = spawn_info.m_client_script
    end

    local entity = _M.CreatePrefab(self, scene, prefab, transform)
    local script = ctx:GetScriptManager():Get(entity)
    if script then
        local go_definition = _M.ConvertItemDefinitionToGODefinition(entity, spawn_info.m_did, definition)
        script:initGameObject(go_definition)

        if script.m_gameobject.m_constructable_component then
            script.m_gameobject.m_constructable_component:StepToNextPhase()
        end
    end

    ctx:Log("spawn item ", spawn_info.m_did,
            " on SpawnPoint(", spawn_info.m_spawn_point_name, ")",
            ", on Layer(", spawn_info.m_spawn_on_layer, ")")

    return entity, script and script.m_gameobject
end

---@param self ClientCreation
---@param scene Scene
---@param character_entity LogicEntity
---@return LogicEntity
function _M:createWeaponController(scene, character_entity)
    local ctx = TL_Client.GetContext()

    local prefab = Creation.ConvertWeaponControllerToPrefab()
    prefab.m_client_script = k_default_script
    local controller_entity = scene:Instantiate(prefab, nil)
    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    local controller_relationship = ctx:GetRelationshipManager():Get(controller_entity)
    if controller_relationship then
        controller_relationship:RemoveFromParent()
        local character_relationship = ctx:GetRelationshipManager():Get(character_entity)
        if character_relationship then
            character_relationship:AddChild(controller_entity)
        end
    end

    return controller_entity
end

---@param self ClientCreation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param net_id number
---@param object_definitions ObjectDefinitionTable
---@param hfsm_definition ScriptHFSMDefinitionHandle|nil
---@return LogicEntity, ClientGameObject
function _M:CreateCharacter(scene, spawn_info, position, net_id, object_definitions, hfsm_definition)
    local ctx = TL_Client.GetContext()

    local transform = TL_Common.Transform()
    transform.m_position = position

    local definition = object_definitions:GetCharacter(spawn_info.m_did)
    local prefab = Creation.ConvertCharacterDefinitionToPrefab(definition)

    if hfsm_definition then
        prefab.m_hfsm = hfsm_definition
    end

    if spawn_info.m_client_script:empty() then
        prefab.m_client_script = k_default_script
    else
        prefab.m_client_script = spawn_info.m_client_script
    end

    local entity = _M.CreatePrefab(self, scene, prefab, transform)

    ctx:Log("spawn character ", spawn_info.m_did,
            " on SpawnPoint(", spawn_info.m_spawn_point_name, ")",
            ", on Layer(", spawn_info.m_spawn_on_layer, ")")

    local controller_entity = TL_Common.null_entity
    if definition.m_init_weapon ~= TL_Schema.DID.Invalid then
        local weapon_spawn_definition = TL_Schema.ObjectSpawnDefinition()
        weapon_spawn_definition.m_did = definition.m_init_weapon
        local weapon_item_entity = _M.CreateItem(self, scene, weapon_spawn_definition, TL_Common.Vec2(0, 0), object_definitions)
        local weapon_def = object_definitions:GetItem(definition.m_init_weapon)

        controller_entity = _M.createWeaponController(self, scene, entity)

        local weapon_item_relationship = ctx:GetRelationshipManager():Get(weapon_item_entity)
        if weapon_item_relationship then
            weapon_item_relationship:RemoveFromParent()
            local controller_relationship = ctx:GetRelationshipManager():Get(controller_entity)
            if controller_relationship then
                controller_relationship:AddChild(weapon_item_entity)
            else
                ctx:Log("weapon controller don't has relationship component")
            end
        else
            ctx:Log("weapon item don't has relationship component")
        end

        local controller_script = ctx:GetScriptManager():Get(controller_entity)
        if controller_script then
            local controller_go_definition = _M.ConvertWeaponControllerToGODefinition(controller_entity, weapon_item_entity, definition.m_init_weapon, weapon_def)
            controller_script:initGameObject(controller_go_definition)
        end
    end

    local script = ctx:GetScriptManager():Get(entity)
    if script then
        local go_definition = _M.ConvertCharacterDefinitionToGODefinition(entity, spawn_info.m_did, definition, controller_entity)
        go_definition.m_net_id = net_id
        script:initGameObject(go_definition)
    end

    for i, skill_did in ipairs(definition.m_skills) do
        if skill_did ~= TL_Schema.DID.Invalid then
            local skill_spawn_info = TL_Schema.ObjectSpawnDefinition()
            skill_spawn_info.m_did = skill_did
            local skill_entity = _M.CreateSkill(self, scene, skill_spawn_info, TL_Common.Vec2(0, 0), object_definitions)

            local character_relationship = ctx:GetRelationshipManager():Get(entity)
            if character_relationship then
                local skill_relationship = ctx:GetRelationshipManager():Get(skill_entity)
                if skill_relationship then
                    skill_relationship:RemoveFromParent()
                end
                character_relationship:AddChild(skill_entity)
            end

            local skill_script = ctx:GetScriptManager():Get(skill_entity)
            if skill_script and skill_script.m_gameobject then
                local skill_go = skill_script.m_gameobject
                if skill_go.m_skill_components and #skill_go.m_skill_components > 0 then
                    local action_name = definition.m_skill_action_binding and definition.m_skill_action_binding[i - 1] or ""
                    -- TODO: store action mapping for runtime use
                end
            end
        end
    end

    return entity, script and script.m_gameobject
end

---@param self ClientCreation
---@param scene Scene
---@param player_related_definition PlayerRelatedDefinition
---@param object_definitions ObjectDefinitionTable
function _M:CreatePlayerHint(scene, player_related_definition, object_definitions)
    local did = player_related_definition.m_hint_fx_did
    if did == TL_Schema.DID.Invalid then
        return
    end
    local fx_definition = object_definitions:GetFX(did)
    TL_Client.GetContext():Log("creating player hint")
    _M.CreatePlayerHintFX(scene, did, fx_definition, k_default_script)
end

return _M
