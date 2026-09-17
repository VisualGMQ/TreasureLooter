local GameObject = require("common.gameobject")

---@class Creation
local _M = {}
_M.__index = _M

--- Convert to prefab without client/server script.
---@param config CharacterDefinitionHandle
---@return PrefabHandle
function _M.ConvertCharacterDefinitionToPrefab(config)
    local ctx = TL_Common.GetContext()
    local prefab_mgr = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_mgr:Create()

    local sprite = TL_Schema.SpriteDefinition()
    sprite.m_image = config.m_sprite_sheet
    sprite.m_region.m_size.x = 16
    sprite.m_region.m_size.y = 16
    prefab_handle.m_sprite = sprite

    prefab_handle.m_draw_order = TL_Schema.DrawOrderDefinition()
    prefab_handle.m_draw_order.m_enable_y_sorting = true
    prefab_handle.m_draw_order.m_draw_layer = TL_Schema.DrawLayer.TilemapArch

    -- move down anim
    local move_anim_player = TL_Schema.AnimationPlayerDefinition()
    move_anim_player.m_animation = config.m_move.m_move_down_animation
    move_anim_player.m_auto_play = false
    move_anim_player.m_loop = -1
    move_anim_player.m_rate = 1.0
    local multi_anim = TL_Schema.MultiAnimationPlayerDefinition()
    local anims = multi_anim.m_animations
    table.insert(anims, move_anim_player)

    -- hurt anim
    local hurt_anim_player = TL_Schema.AnimationPlayerDefinition()
    hurt_anim_player.m_loop = 0
    hurt_anim_player.m_rate = 1.0
    hurt_anim_player.m_auto_play = false
    hurt_anim_player.m_animation = config.m_hp.m_hurt_animation
    table.insert(anims, hurt_anim_player)

    multi_anim.m_animations = anims
    prefab_handle.m_animations = multi_anim

    prefab_handle.m_cct = config.m_cct

    if config.m_interact then
        prefab_handle.m_trigger = _M.buildInteractTrigger(config.m_interact.m_detect_range, config.m_interact.m_collision_mask)
    end

    return prefab_handle
end

--- Build the interact detect trigger in lua: a circle of radius = detect_range with the given mask.
---@param detect_range number
---@param collision_mask CollisionGroup
---@return TriggerDefinition
function _M.buildInteractTrigger(detect_range, collision_mask)
    local ctx = TL_Common.GetContext()
    local shape = ctx:GetAssetsManager():GetPhysicsShapeDefinitionManager():Create()
    shape.m_is_rect = false

    local circle = shape.m_circle
    circle.m_center = TL_Common.Vec2(0, 0)
    circle.m_radius = detect_range
    shape.m_circle = circle

    shape.m_collision_mask = collision_mask

    local trigger = TL_Schema.TriggerDefinition()
    trigger.m_event_type = TL_Schema.TriggerEventType.InteractableDetect
    local shapes = trigger.m_physics_shapes
    table.insert(shapes, shape)
    trigger.m_physics_shapes = shapes
    trigger.m_trig_every_frame_when_touch = false

    return trigger
end

--- Convert a fx definition to prefab (sprite + draw order + one animation to play).
---@param config FXDefinitionHandle
---@return PrefabHandle
function _M.ConvertFXDefinitionToPrefab(config)
    local ctx = TL_Common.GetContext()
    local prefab_mgr = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_mgr:Create()

    prefab_handle.m_transform = TL_Common.Transform()
    prefab_handle.m_sprite = config.m_sprite

    prefab_handle.m_draw_order = TL_Schema.DrawOrderDefinition()
    prefab_handle.m_draw_order.m_enable_y_sorting = true
    prefab_handle.m_draw_order.m_draw_layer = TL_Schema.DrawLayer.TilemapArch

    local multi_anim = TL_Schema.MultiAnimationPlayerDefinition()
    local anims = multi_anim.m_animations
    table.insert(anims, config.m_animation)
    multi_anim.m_animations = anims
    prefab_handle.m_animations = multi_anim

    return prefab_handle
end

--- Convert a skill definition to prefab (sprite + draw order + animation).
---@param config SkillDefinitionHandle
---@return PrefabHandle
function _M.ConvertSkillDefinitionToPrefab(config)
    local ctx = TL_Common.GetContext()
    local prefab_mgr = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_mgr:Create()

    prefab_handle.m_transform = TL_Common.Transform()
    prefab_handle.m_sprite = TL_Schema.SpriteDefinition()
    prefab_handle.m_sprite.m_region.m_size = TL_Common.Vec2(1, 1)
    prefab_handle.m_sprite.m_size = TL_Common.Vec2(1, 1)

    prefab_handle.m_draw_order = TL_Schema.DrawOrderDefinition()
    prefab_handle.m_draw_order.m_enable_y_sorting = true
    prefab_handle.m_draw_order.m_draw_layer = TL_Schema.DrawLayer.TilemapArch

    local multi_anim = TL_Schema.MultiAnimationPlayerDefinition()
    local anims = multi_anim.m_animations
    if #config.m_phases > 0 then
        local anim_player_def = TL_Schema.AnimationPlayerDefinition()
        anim_player_def.m_animation = config.m_phases[1].m_animation
        anim_player_def.m_auto_play = false
        anim_player_def.m_loop = 0
        anim_player_def.m_rate = 1.0
        table.insert(anims, anim_player_def)
    end
    multi_anim.m_animations = anims
    prefab_handle.m_animations = multi_anim

    local merged_trigger = TL_Schema.TriggerDefinition()
    local merged_shapes = merged_trigger.m_physics_shapes
    for _, phase in ipairs(config.m_phases) do
        if phase.m_collision and phase.m_collision.m_collision then
            for _, shape in ipairs(phase.m_collision.m_collision.m_physics_shapes) do
                table.insert(merged_shapes, shape)
            end
        end
    end
    if #merged_shapes > 0 then
        merged_trigger.m_physics_shapes = merged_shapes
        prefab_handle.m_trigger = merged_trigger
    end

    return prefab_handle
end

--- Convert to prefab without client/server script. only the item itself, no controller.
---@param config ItemDefinitionHandle
---@return PrefabHandle
function _M.ConvertItemDefinitionToPrefab(config)
    return _M.convertItemDefinitionToPrefab(config)
end

--- The weapon controller is a character-owned entity that holds & drives a weapon item.
---@return PrefabHandle
function _M.ConvertWeaponControllerToPrefab()
    local ctx = TL_Common.GetContext()
    local prefab_mgr = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_mgr:Create()

    prefab_handle.m_transform = TL_Common.Transform()
    prefab_handle.m_name = "weapon_controller"

    prefab_handle.m_draw_order = TL_Schema.DrawOrderDefinition()
    prefab_handle.m_draw_order.m_enable_y_sorting = true
    prefab_handle.m_draw_order.m_draw_layer = TL_Schema.DrawLayer.TilemapArch

    return prefab_handle
end

---@param config ItemDefinitionHandle
---@return PrefabHandle
function _M.convertItemDefinitionToPrefab(config)
    local ctx = TL_Common.GetContext()
    local prefab_mgr = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_mgr:Create()

    prefab_handle.m_transform = TL_Common.Transform()
    prefab_handle.m_name = "weapon"

    prefab_handle.m_sprite = config.m_sprite

    prefab_handle.m_draw_order = TL_Schema.DrawOrderDefinition()
    prefab_handle.m_draw_order.m_enable_y_sorting = true
    prefab_handle.m_draw_order.m_draw_layer = TL_Schema.DrawLayer.TilemapArch

    if config.m_collision:IsValid() then
        local static_collision = TL_Schema.StaticCollisionDefinition()
        local collisions = static_collision.m_collisions
        table.insert(collisions, config.m_collision)
        static_collision.m_collisions = collisions
        prefab_handle.m_static_collision = static_collision
    end

    if config.m_attackable then
        local anim_player = TL_Schema.AnimationPlayerDefinition()
        anim_player.m_animation = config.m_attackable.m_attack_animation
        anim_player.m_auto_play = false
        anim_player.m_loop = 0
        anim_player.m_rate = 1.0
        local multi_anim = TL_Schema.MultiAnimationPlayerDefinition()
        local anim_list = multi_anim.m_animations
        table.insert(anim_list, anim_player)
        multi_anim.m_animations = anim_list
        prefab_handle.m_animations = multi_anim

        local trigger_definition = TL_Schema.TriggerDefinition()

        -- TODO: improve the C++ array & vector operation
        local shapes = trigger_definition.m_physics_shapes
        table.insert(shapes, config.m_attackable.m_collision)
        trigger_definition.m_physics_shapes = shapes

        trigger_definition.m_trig_every_frame_when_touch = false
        trigger_definition.m_event_type = TL_Schema.TriggerEventType.WeaponAttack
        prefab_handle.m_trigger = trigger_definition
    end

    return prefab_handle
end

-- create entity functions, overridden by client/server Creation

---@param self Creation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param net_id number
---@param object_definitions ObjectDefinitionTable
---@param hfsm_definition ScriptHFSMDefinitionHandle|nil
---@return LogicEntity, GameObject
function _M:CreateCharacter(scene, spawn_info, position, net_id, object_definitions, hfsm_definition)
    error("shouldn't step into here")
end

---@param self Creation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param object_definitions ObjectDefinitionTable
---@return LogicEntity, GameObject
function _M:CreateItem(scene, spawn_info, position, object_definitions)
    error("shouldn't step into here")
end

---@param self Creation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param object_definitions ObjectDefinitionTable
---@return LogicEntity, GameObject
function _M:CreateFX(scene, spawn_info, position, object_definitions)
    error("shouldn't step into here")
end

---@param self Creation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param object_definitions ObjectDefinitionTable
---@return LogicEntity, GameObject
function _M:CreateSkill(scene, spawn_info, position, object_definitions)
    error("shouldn't step into here")
end

---@param self Creation
---@param scene Scene
---@param prefab PrefabHandle
---@param transform Transform
---@return LogicEntity
function _M:CreatePrefab(scene, prefab, transform)
    error("shouldn't step into here")
end

---@param self Creation
---@param scene Scene
---@param player_related_definition PlayerRelatedDefinition
---@param object_definitions ObjectDefinitionTable
function _M:CreatePlayerHint(scene, player_related_definition, object_definitions)
    error("shouldn't step into here")
end

return _M
