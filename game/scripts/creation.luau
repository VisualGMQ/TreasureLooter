local TL = require("@game/tl_types")
local CharacterBehavior = require("@game/character_behavior")
local WeaponBehavior = require("@game/weapon_behavior")

local _M = {}

function _M.createHitShape(config: TL.TriggerDefinition) : TL.Entity
    local ctx: TL.GameContext = TL.GetContext()

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return TL.null_entity
    end

    local prefab_mgr = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_mgr:Create()

    if not prefab_handle:IsValid() then
        return TL.null_entity
    end

    prefab_handle.m_transform = TL.Transform()
    if config.m_physics_actor.m_is_rect then
        prefab_handle.m_transform.m_position = config.m_physics_actor.m_rect.m_center
    else
        prefab_handle.m_transform.m_position = config.m_physics_actor.m_circle.m_center
    end

    TL.Log("position = ", prefab_handle.m_transform.m_position)

    local trigger_def : TL.TriggerDefinition = TL.TriggerDefinition()
    trigger_def.m_event_type = config.m_event_type
    trigger_def.m_trig_every_frame_when_touch = config.m_trig_every_frame_when_touch
    trigger_def.m_physics_actor = config.m_physics_actor
    prefab_handle.m_trigger = trigger_def

    local entity = scene:Instantiate(prefab_handle)
    prefab_mgr:Unload(prefab_handle)

    return entity
end

function _M.CreateWeapon(definition: TL.WeaponDefinitionHandle) : WeaponBehavior.WeaponBehavior?
    if not definition:IsValid() then
        return nil
    end

    local ctx: TL.GameContext = TL.GetContext()

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return nil
    end

    local prefab_manager = ctx:GetAssetsManager():GetPrefabManager()
    -- create control node
    local prefab_handle = prefab_manager:Create()

    prefab_handle.m_transform = TL.Transform()
    prefab_handle.m_script = TL.Path("scripts/weapon_behavior.luau")
    local control_entity = scene:Instantiate(prefab_handle)
    prefab_manager:Unload(prefab_handle)

    -- fill weapon prefab
    prefab_handle = prefab_manager:Create()
    if not prefab_handle:IsValid() then
        return nil
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


    prefab_handle.m_bind_points = definition.m_bind_points

    local weapon_entity = scene:Instantiate(prefab_handle)
    prefab_manager:Unload(prefab_handle)

    -- put weapon entity under control node

    ctx:GetRelationshipManager():RegisterEntity(control_entity)
    local control_relationship = ctx:GetRelationshipManager():Get(control_entity)
    control_relationship:AddChild(weapon_entity)

    -- create hit shape as child entity
    local relationship_mgr = ctx:GetRelationshipManager()
    relationship_mgr:RegisterEntity(weapon_entity)
    local relationship = relationship_mgr:Get(weapon_entity)

    for i, shape in ipairs(definition.m_hit_shapes) do
        TL.Log("create hit shape ", i)
        local shape_entity = _M.createHitShape(shape)
        if shape_entity == TL.null_entity then
            TL.Log("create hit shape entity ", i , " failed")
        else
            relationship:AddChild(shape_entity)
        end
    end


    -- init script by definition
    local script = ctx:GetScriptManager():Get(control_entity) :: WeaponBehavior.WeaponBehavior

    if not script then
        TL.Log("no script binding")
        return nil
    end

    script:_initFromDefinition(definition)

    return script
end

function _M.CreateCharacter(transform: TL.Transform, definition: TL.CharacterDefinitionHandle) : CharacterBehavior.CharacterBehavior?
    if not definition:IsValid()  then
        TL.Log("invalid character config")
        return nil
    end

    local ctx: TL.GameContext = TL.GetContext()

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        TL.Log("scene not exists")
        return nil
    end

    local prefab = ctx:GetAssetsManager():GetPrefabManager():Create()

    if not prefab:IsValid() then
        return nil
    end

    -- create weapon as child entity
    local weapon = nil
    if definition.m_weapon ~= nil then
        weapon = _M.CreateWeapon(definition.m_weapon)
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

    local physics_actor_info_mgr = ctx:GetAssetsManager():GetPhysicsActorInfoManager()
    prefab_handle.m_cct = definition.m_cct

    local entity = scene:Instantiate(prefab_handle)
    physics_actor_info_mgr:Unload(prefab_handle.m_cct.m_physics_actor)
    prefab_manager:Unload(prefab_handle)

    -- set weapon as children
    if weapon and weapon:GetEntity() ~= TL.null_entity then
        ctx:GetRelationshipManager():RegisterEntity(entity)
        local relationship = ctx:GetRelationshipManager():Get(entity)
        relationship:AddChild(weapon:GetEntity())
    end

    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    local script = (ctx:GetScriptManager():Get(entity) :: CharacterBehavior.CharacterBehavior)
    if script then
        script:_initByDefinition(definition, weapon)
    end

    return script
end



return _M
