local TL = require("@game/tl_types")
local Character = require("@game/character")
local Weapon = require("@game/weapon")
local Item = require("@game/item")

local _M = {}

function _M.CreateWeapon(definition: TL.WeaponDefinitionHandle) : (TL.Entity, Weapon.Weapon?)
    if not definition:IsValid() then
        return TL.null_entity
    end

    local ctx: TL.GameContext = TL.GetContext()

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return TL.null_entity
    end

    local prefab_manager = ctx:GetAssetsManager():GetPrefabManager()
    -- create control node
    local prefab_handle = prefab_manager:Create()

    prefab_handle.m_transform = TL.Transform()
    local control_entity = scene:Instantiate(prefab_handle)
    prefab_manager:Unload(prefab_handle)

    -- fill weapon prefab
    prefab_handle = prefab_manager:Create()
    if not prefab_handle:IsValid() then
        return TL.null_entity
    end

    prefab_handle.m_transform = TL.Transform()
    prefab_handle.m_sprite = TL.Sprite()
    prefab_handle.m_sprite.m_image = definition.m_sprite
    prefab_handle.m_sprite.m_size = definition.m_sprite:GetSize()
    prefab_handle.m_sprite.m_region.m_size = definition.m_sprite:GetSize()

    prefab_handle.m_draw_order = TL.DrawOrderDefinition()
    prefab_handle.m_draw_order.m_enable_y_sorting = false
    prefab_handle.m_draw_order.m_draw_layer = TL.DrawLayer.TilemapArch

    local anim_mgr = ctx:GetAssetsManager():GetAnimationManager()
    prefab_handle.m_animation = TL.AnimationPlayerDefinition()
    prefab_handle.m_animation.m_animation = anim_mgr:Load("assets/gpa/anim/item/weapon_forward_attack.animation.xml", false)
    prefab_handle.m_animation.m_auto_play = false
    prefab_handle.m_animation.m_loop = 0
    prefab_handle.m_animation.m_rate = 1.0

    prefab_handle.m_trigger = definition.m_hit_shapes

    prefab_handle.m_bind_points = definition.m_bind_points

    local weapon_entity = scene:Instantiate(prefab_handle)
    prefab_manager:Unload(prefab_handle)

    -- put weapon entity under control node

    local control_relationship = ctx:GetRelationshipManager():Get(control_entity)
    control_relationship:AddChild(weapon_entity)

    -- init script by definition
    return control_entity, Weapon.new(control_entity, definition)
end

function _M.CreateCharacter(transform: TL.Transform, definition: TL.CharacterDefinitionHandle) : (TL.Entity, Character.Character?)
    if not definition:IsValid()  then
        TL.Log("invalid character config")
        return TL.null_entity
    end

    local ctx: TL.GameContext = TL.GetContext()

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        TL.Log("scene not exists")
        return TL.null_entity
    end

    local prefab = ctx:GetAssetsManager():GetPrefabManager():Create()

    if not prefab:IsValid() then
        return TL.null_entity
    end

    -- create weapon as child entity
    local weapon = nil
    local weapon_entity = TL.null_entity
    if definition.m_weapon ~= nil then
        weapon_entity, weapon = _M.CreateWeapon(definition.m_weapon)
    end

    -- fill prefab
    local prefab_manager = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_manager:Create()

    prefab_handle.m_transform = transform
    prefab_handle.m_script = definition.m_script
    prefab_handle.m_sprite = TL.Sprite()
    prefab_handle.m_sprite.m_image = definition.m_sprite_sheet
    prefab_handle.m_sprite.m_size.x = 16
    prefab_handle.m_sprite.m_size.y = 16
    prefab_handle.m_sprite.m_region.m_size.x = 16
    prefab_handle.m_sprite.m_region.m_size.y = 16

    prefab_handle.m_draw_order = TL.DrawOrderDefinition()
    prefab_handle.m_draw_order.m_enable_y_sorting = false
    prefab_handle.m_draw_order.m_draw_layer = TL.DrawLayer.TilemapArch

    prefab_handle.m_animation = TL.AnimationPlayerDefinition()
    prefab_handle.m_animation.m_animation = definition.m_move_down_animation
    prefab_handle.m_animation.m_auto_play = true
    prefab_handle.m_animation.m_loop = -1
    prefab_handle.m_animation.m_rate = 1.0

    local shape_definition = nil
    local shape_definition_mgr = ctx:GetAssetsManager():GetPhysicsShapeDefinitionManager()
    if definition.m_interact_range > 0 then
        local trigger_definition: TL.TriggerDefinition = TL.TriggerDefinition()
        trigger_definition.m_event_type = TL.TriggerEventType.InteractableDetect
        trigger_definition.m_trig_every_frame_when_touch = false
        shape_definition = shape_definition_mgr:Create()
        shape_definition.m_is_rect = false
        shape_definition.m_circle.m_center = TL.Vec2.ZERO
        shape_definition.m_circle.m_radius = definition.m_interact_range
        shape_definition.m_collision_layer:Add(TL.CollisionGroupType.Interactable)
        shape_definition.m_collision_mask:Add(TL.CollisionGroupType.Interactable)

        -- FIXME: we need register std::vector
        local shapes = trigger_definition.m_physics_shapes
        table.insert(shapes, shape_definition)
        trigger_definition.m_physics_shapes = shapes
        prefab_handle.m_trigger = trigger_definition
    end

    local physics_shape_info_mgr = ctx:GetAssetsManager():GetPhysicsShapeDefinitionManager()
    prefab_handle.m_cct = definition.m_cct

    local entity = scene:Instantiate(prefab_handle)
    physics_shape_info_mgr:Unload(prefab_handle.m_cct.m_physics_shape)
    prefab_manager:Unload(prefab_handle)
    if shape_definition then
        shape_definition_mgr:Unload(shape_definition)
    end

    -- set weapon as children
    if weapon and weapon_entity ~= TL.null_entity then
        local relationship = ctx:GetRelationshipManager():Get(entity)
        relationship:AddChild(weapon_entity)
    end

    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    return entity, Character.new(entity, definition, weapon)
end

-- return value:
-- index 0 : TL.Entity
-- index 1 : ItemBehavior.ItemBehavior
function _M.CreateItem(transform: TL.Transform, definition: TL.ItemDefinitionHandle): (TL.Entity, Item.Item)
    if not definition:IsValid()  then
        TL.Log("invalid item definition")
        return TL.null_entity
    end

    local ctx: TL.GameContext = TL.GetContext()

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        TL.Log("scene not exists")
        return TL.null_entity
    end

    local prefab = ctx:GetAssetsManager():GetPrefabManager():Create()

    if not prefab:IsValid() then
        TL.Log("prefab invalid")
        return TL.null_entity
    end

    -- fill prefab
    local prefab_manager = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_manager:Create()

    prefab_handle.m_transform = transform
    prefab_handle.m_sprite = TL.Sprite()
    prefab_handle.m_sprite = definition.m_sprite

    prefab_handle.m_draw_order = TL.DrawOrderDefinition()
    prefab_handle.m_draw_order.m_enable_y_sorting = false
    prefab_handle.m_draw_order.m_draw_layer = TL.DrawLayer.TilemapArch

    local collisions = prefab_handle.m_static_collision.m_collisions
    table.insert(collisions, definition.m_collision)
    prefab_handle.m_static_collision.m_collisions = collisions

    local entity = scene:Instantiate(prefab_handle)
    prefab_manager:Unload(prefab_handle)

    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    return entity, Item.new(entity, definition)
end


return _M
