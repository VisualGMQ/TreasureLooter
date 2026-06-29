local _M = {}
_M.__index = _M


-- convert to prefab without client/server script
function _M.ConvertCharacterDefinitionToPrefab(config:CharacterDefinitionHandle):PrefabHandle
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

-- build the interact detect trigger in lua: a circle of radius = detect_range with the given mask
function _M.buildInteractTrigger(detect_range:number, collision_mask:CollisionGroup):TriggerDefinition
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

-- convert a fx definition to prefab (sprite + draw order + one animation to play)
function _M.ConvertFXDefinitionToPrefab(config:FXDefinitionHandle):PrefabHandle
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

-- convert to prefab without client/server script. only the item itself, no controller.
function _M.ConvertItemDefinitionToPrefab(config:ItemDefinitionHandle):PrefabHandle
    return _M.convertItemDefinitionToPrefab(config)
end

-- the weapon controller is a character-owned entity that holds & drives a weapon item
function _M.ConvertWeaponControllerToPrefab():PrefabHandle
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

function _M.convertItemDefinitionToPrefab(config:ItemDefinitionHandle):PrefabHandle
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

function _M.CreateCharacter(self, scene:Scene, spawn_info:ObjectSpawnDefinition, position:Vec2, object_definitions): Entity
    assert(false, "shouldn't step into here")
end

function _M.CreateItem(self, scene:Scene, spawn_info:ObjectSpawnDefinition, position:Vec2, object_definitions): Entity
    assert(false, "shouldn't step into here")
end

function _M.CreateFX(self, scene:Scene, spawn_info:ObjectSpawnDefinition, position:Vec2, object_definitions): Entity
    assert(false, "shouldn't step into here")
end

function _M.CreatePrefab(self, scene:Scene, prefab:PrefabHandle, transform:Transform): Entity
    assert(false, "shouldn't step into here")
end

function _M.CreatePlayerHint(self, scene:Scene, player_related_definition:PlayerRelatedDefinition, object_definitions)
    assert(false, "shouldn't step into here")
end

return _M