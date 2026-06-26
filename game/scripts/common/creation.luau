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
    move_anim_player.m_animation = config.m_move_down_animation
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
    hurt_anim_player.m_animation = config.m_hurt_animation
    table.insert(anims, hurt_anim_player)

    multi_anim.m_animations = anims
    prefab_handle.m_animations = multi_anim

    prefab_handle.m_cct = config.m_cct
    return prefab_handle
end

-- convert to prefab without client/server script
function _M.ConvertItemDefinitionToPrefab(config:ItemDefinitionHandle):PrefabHandle
    local item_prefab = _M.convertItemWeaponDefinitionToPrefab(config)
    local item_controller_prefab = _M.convertItemWeaponControlDefinitionToPrefab(config)

    local children = item_controller_prefab.m_children
    table.insert(children, item_prefab)
    item_controller_prefab.m_children = children

    return item_controller_prefab
end

function _M.convertItemWeaponControlDefinitionToPrefab(config:ItemDefinitionHandle):PrefabHandle
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

function _M.convertItemWeaponDefinitionToPrefab(config:ItemDefinitionHandle):PrefabHandle
    local ctx = TL_Common.GetContext()
    local prefab_mgr = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_mgr:Create()

    prefab_handle.m_transform = TL_Common.Transform()
    prefab_handle.m_name = "weapon"

    prefab_handle.m_sprite = config.m_sprite

    prefab_handle.m_draw_order = TL_Schema.DrawOrderDefinition()
    prefab_handle.m_draw_order.m_enable_y_sorting = true
    prefab_handle.m_draw_order.m_draw_layer = TL_Schema.DrawLayer.TilemapArch

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

return _M