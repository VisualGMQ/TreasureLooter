local TL = require("@game/tl_types")
local Config = require("@game/config")
local CharacterBehavior = require("@game/character_behavior")
local WeaponBehavior = require("@game/weapon_behavior")

local _M = {}

function _M.createHitShape(config: TL.TriggerDefinition) : TL.Entity
    local ctx: TL.GameContext = TL.GetContext()

    local level = ctx:GetLevelManager():GetCurrentLevel()
    if not level then
        return TL.null_entity
    end

    local prefab_mgr = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_mgr:Create()

    if not prefab_handle:IsValid() then
        return TL.null_entity
    end

    prefab_handle.m_transform = TL.Transform()

    local trigger_def : TL.TriggerDefinition = TL.TriggerDefinition()
    trigger_def.m_event_type = config.m_event_type
    trigger_def.m_trig_every_frame_when_touch = config.m_trig_every_frame_when_touch
    trigger_def.m_physics_actor = config.m_physics_actor
    trigger_def.m_bind_point_name = config.m_bind_point_name
    prefab_handle.m_trigger = trigger_def

    local entity = level:Instantiate(prefab_handle)
    prefab_mgr:Unload(prefab_handle)

    return entity
end

function _M.CreateWeapon(config: TL.WeaponDefinitionHandle) : Config.WeaponConfig?
    if not config:IsValid() then
        return nil
    end

    local self = {} :: Config.WeaponConfig

    local ctx: TL.GameContext = TL.GetContext()

    local level = ctx:GetLevelManager():GetCurrentLevel()
    if not level then
        return self
    end

    -- fill member properties
    self.m_type = config.m_type
    self.m_image = config.m_sprite
    self.m_cooldown = config.m_cooldown
    self.m_attack_speed = config.m_attack_speed
    -- self.m_attack_regions = config.m_attack_regions

    -- fill prefab
    local prefab_manager = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_manager:Create()
    if not prefab_handle:IsValid() then
        return self
    end

    prefab_handle.m_transform = TL.Transform()
    prefab_handle.m_script = TL.Path("scripts/weapon_behavior.luau")
    prefab_handle.m_sprite = TL.Sprite()
    prefab_handle.m_sprite.m_image = self.m_image

    -- prefab_handle.m_sprite.m_size.x = 0
    -- prefab_handle.m_sprite.m_size.y = 0
    -- prefab_handle.m_sprite.m_region.m_size.x = 0
    -- prefab_handle.m_sprite.m_region.m_size.y = 0

    local anim_mgr = ctx:GetAssetsManager():GetAnimationManager()
    prefab_handle.m_animation = TL.AnimationPlayerDefinition()
    prefab_handle.m_animation.m_animation = anim_mgr:Load("assets/gpa/anim/item/weapon_forward_attack.animation.xml", false)
    prefab_handle.m_animation.m_auto_play = false
    prefab_handle.m_animation.m_loop = 0
    prefab_handle.m_animation.m_rate = 1.0

    prefab_handle.m_bind_points = config.m_bind_points

    self.m_entity = level:Instantiate(prefab_handle)
    prefab_manager:Unload(prefab_handle)

    local relationship_mgr = ctx:GetRelationshipManager()
    relationship_mgr:RegisterEntity(self.m_entity)
    local relationship = relationship_mgr:Get(self.m_entity)
    
    -- create hit shape as child entity
    for i, shape in ipairs(config.m_hit_shapes) do
        local entity = _M.createHitShape(shape)
        if entity == TL.null_entity then
            TL.Log("create hit shape entity ", i , " failed")
        else
            local children = relationship.m_children
            children[#children + 1] = entity
            relationship.m_children = children
        end
    end

    local script = ctx:GetScriptManager():Get(self.m_entity)
    if script then
        (script :: WeaponBehavior.WeaponBehavior):__attach_config(config)
    end

    return self
end

function _M.CreateCharacter(transform: TL.Transform, config: TL.CharacterDefinitionHandle) : Config.CharacterConfig?
    if not config:IsValid()  then
        TL.Log("invalid character config")
        return nil
    end

    local self = {} :: Config.CharacterConfig

    local ctx: TL.GameContext = TL.GetContext()

    local level = ctx:GetLevelManager():GetCurrentLevel()
    if not level then
        return self
    end

    local prefab = ctx:GetAssetsManager():GetPrefabManager():Create()

    if not prefab:IsValid() then
        return self
    end

    -- fill member properties
    self.m_move_speed = config.m_speed
    self.m_move_left_anim = config.m_move_left_animation
    self.m_move_right_anim = config.m_move_right_animation
    self.m_move_up_anim = config.m_move_up_animation
    self.m_move_down_anim = config.m_move_down_animation

    -- create weapon as child entity
    self.m_weapon_entity = TL.null_entity
    if config.m_weapon ~= nil then
        local weapon = _M.CreateWeapon(config.m_weapon)
        if weapon then
            self.m_weapon_entity = weapon.m_entity
            if self.m_weapon_entity == TL.null_entity then
                TL.Log("failed to create weapon")
            end
        end
    end

    -- fill prefab
    local ctx : TL.GameContext = TL.GetContext()
    local prefab_manager = ctx:GetAssetsManager():GetPrefabManager()
    local prefab_handle = prefab_manager:Create()

    prefab_handle.m_transform = transform
    prefab_handle.m_script = config.m_script
    prefab_handle.m_sprite = TL.Sprite()
    prefab_handle.m_sprite.m_image = config.m_sprite_sheet
    prefab_handle.m_sprite.m_size.x = 16
    prefab_handle.m_sprite.m_size.y = 16
    prefab_handle.m_sprite.m_region.m_size.x = 16
    prefab_handle.m_sprite.m_region.m_size.y = 16

    prefab_handle.m_animation = TL.AnimationPlayerDefinition()
    prefab_handle.m_animation.m_animation = config.m_move_down_animation
    prefab_handle.m_animation.m_auto_play = true
    prefab_handle.m_animation.m_loop = -1
    prefab_handle.m_animation.m_rate = 1.0

    local physics_actor_info_mgr = ctx:GetAssetsManager():GetPhysicsActorInfoManager()
    prefab_handle.m_cct = config.m_cct

    self.m_entity = level:Instantiate(prefab_handle)
    physics_actor_info_mgr:Unload(prefab_handle.m_cct.m_physics_actor)
    prefab_manager:Unload(prefab_handle)

    -- set weapon as children
    if self.m_weapon_entity ~= TL.null_entity then
        ctx:GetRelationshipManager():RegisterEntity(self.m_entity)
        local relationship = ctx:GetRelationshipManager():Get(self.m_entity)
        local children = relationship.m_children
        children[#children + 1] = self.m_weapon_entity
        relationship.m_children = children
    end

    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    local script = ctx:GetScriptManager():Get(self.m_entity)
    if script then
        (script :: CharacterBehavior.CharacterBehavior):__attach_config(self, self.m_weapon_entity)
    end
    
    return self
end



return _M
