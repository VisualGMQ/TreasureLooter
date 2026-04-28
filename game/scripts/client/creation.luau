local TL_Common = require("@type_hints/tl_common_types")
local TL_Client = require("@type_hints/tl_client_types")
local TL_Schema = require("@type_hints/tl_schema_types")


local Character = require("@client/character")
local Weapon = require("@client/weapon")
local ItemBehavior = require("@client/item_behavior")

local _M = {}

function _M.CreateWeapon(definition: TL_Schema.WeaponDefinitionHandle) : (TL_Common.Entity, Weapon.Weapon?)
    if not definition:IsValid() then
        return TL_Common.null_entity
    end

    local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return TL_Common.null_entity
    end

    local prefab_manager = ctx:GetAssetsManager():GetPrefabManager()
    -- create control node
    local prefab_handle = prefab_manager:Create()

    prefab_handle.m_transform = TL_Common.Transform()
    local control_entity = scene:Instantiate(prefab_handle)
    prefab_manager:Unload(prefab_handle)

    -- fill weapon prefab
    prefab_handle = prefab_manager:Create()
    if not prefab_handle:IsValid() then
        return TL_Common.null_entity
    end

    prefab_handle.m_transform = TL_Common.Transform()
    prefab_handle.m_sprite = definition.m_sprite

    prefab_handle.m_draw_order = TL_Client.DrawOrderDefinition()
    prefab_handle.m_draw_order.m_enable_y_sorting = false
    prefab_handle.m_draw_order.m_draw_layer = TL_Schema.DrawLayer.TilemapArch

    prefab_handle.m_animation = TL_Client.AnimationPlayerDefinition()
    prefab_handle.m_animation.m_animation = definition.m_attack_animation
    prefab_handle.m_animation.m_auto_play = false
    prefab_handle.m_animation.m_loop = 0
    prefab_handle.m_animation.m_rate = 1.0

    prefab_handle.m_trigger = definition.m_hit_shapes

    local weapon_entity = scene:Instantiate(prefab_handle)
    prefab_manager:Unload(prefab_handle)

    -- put weapon entity under control node

    local control_relationship = ctx:GetRelationshipManager():Get(control_entity)
    control_relationship:AddChild(weapon_entity)

    -- init script by definition
    return control_entity, Weapon.new(control_entity, definition)
end

function _M.CreateCharacter(transform: TL_Common.Transform, definition: TL_Schema.CharacterDefinitionHandle, player_related_definition: TL_Schema.PlayerRelatedDefinition) : (TL_Common.Entity, Character.Character?)
    if not definition:IsValid()  then
        TL_Common.Log("invalid character config")
        return TL_Common.null_entity
    end

    local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        TL_Common.Log("scene not exists")
        return TL_Common.null_entity
    end

    local prefab = ctx:GetAssetsManager():GetPrefabManager():Create()

    if not prefab:IsValid() then
        return TL_Common.null_entity
    end

    -- create weapon as child entity
    local weapon = nil
    local weapon_entity = TL_Common.null_entity
    if definition.m_weapon ~= nil then
        -- weapon_entity, weapon = _M.CreateWeapon(definition.m_weapon)
    end

    local pickaxe = nil
    local pickaxe_entity = TL_Common.null_entity
    if definition.m_pickaxe ~= nil then
        -- pickaxe_entity, pickaxe = _M.CreateWeapon(definition.m_pickaxe)
    end

    -- fill prefab
    local prefab_manager = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_manager:Create()

    prefab_handle.m_transform = transform
    prefab_handle.m_script = definition.m_script
    prefab_handle.m_sprite = TL_Client.Sprite()
    prefab_handle.m_sprite.m_image = definition.m_sprite_sheet
    prefab_handle.m_sprite.m_size.x = 16
    prefab_handle.m_sprite.m_size.y = 16
    prefab_handle.m_sprite.m_region.m_size.x = 16
    prefab_handle.m_sprite.m_region.m_size.y = 16

    prefab_handle.m_draw_order = TL_Schema.DrawOrderDefinition()
    prefab_handle.m_draw_order.m_enable_y_sorting = false
    prefab_handle.m_draw_order.m_draw_layer = TL_Schema.DrawLayer.TilemapArch

    prefab_handle.m_animation = TL_Schema.AnimationPlayerDefinition()
    prefab_handle.m_animation.m_animation = definition.m_move_down_animation
    prefab_handle.m_animation.m_auto_play = true
    prefab_handle.m_animation.m_loop = -1
    prefab_handle.m_animation.m_rate = 1.0

    local physics_shape_info_mgr = ctx:GetAssetsManager():GetPhysicsShapeDefinitionManager()
    prefab_handle.m_cct = definition.m_cct

    local entity = scene:Instantiate(prefab_handle)
    physics_shape_info_mgr:Unload(prefab_handle.m_cct.m_physics_shape)
    prefab_manager:Unload(prefab_handle)

    -- set weapon as children
    local relationship = ctx:GetRelationshipManager():Get(entity)
    if weapon and weapon_entity ~= TL_Common.null_entity then
        relationship:AddChild(weapon_entity)
    end

    if pickaxe and pickaxe_entity ~= TL_Common.null_entity then
        relationship:AddChild(pickaxe_entity)
    end

    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    return entity, Character.new(entity, definition, weapon, pickaxe)
end

-- return value:
-- index 0 : TL_Common.Entity
-- index 1 : ItemBehavior.ItemBehavior
function _M.CreateItem(transform: TL_Common.Transform, definition: TL_Schema.ItemDefinitionHandle): (TL_Common.Entity, ItemBehavior.Item?)
    if not definition:IsValid()  then
        TL_Common.Log("invalid item definition")
        return TL_Common.null_entity
    end

    local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        TL_Common.Log("scene not exists")
        return TL_Common.null_entity
    end

    local prefab = ctx:GetAssetsManager():GetPrefabManager():Create()

    if not prefab:IsValid() then
        TL_Common.Log("prefab invalid")
        return TL_Common.null_entity
    end

    -- fill prefab
    local prefab_manager = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_manager:Create()

    prefab_handle.m_transform = transform
    prefab_handle.m_sprite = TL_Client.Sprite()
    prefab_handle.m_sprite = definition.m_sprite

    prefab_handle.m_draw_order = TL_Schema.DrawOrderDefinition()
    prefab_handle.m_draw_order.m_enable_y_sorting = false
    prefab_handle.m_draw_order.m_draw_layer = TL_Schema.DrawLayer.TilemapArch

    prefab_handle.m_script = definition.m_script

    local collisions = prefab_handle.m_static_collision.m_collisions
    table.insert(collisions, definition.m_collision)
    prefab_handle.m_static_collision.m_collisions = collisions

    local entity = scene:Instantiate(prefab_handle)
    prefab_manager:Unload(prefab_handle)

    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    return entity, ItemBehavior.Item.new(definition)
end

return _M


