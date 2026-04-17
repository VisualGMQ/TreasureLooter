local TL = require("@game/tl_types")
local ScriptBehavior = require("@game/script_behavior")

local WeaponBehavior = {}
WeaponBehavior.__index = WeaponBehavior
setmetatable(WeaponBehavior, { __index = ScriptBehavior })

--[[
Weapon node tree:

---- weapon controler (for play attack animation, WeaponBehavior's entity)
  |---- weapon sprite
    |---- weapon collision_1
    |---- weapon collision_i
    |---- weapon collision_n
--]]

type WeaponBehaviorData = {
    m_type : TL.WeaponType,
    m_image : TL.ImageHandle,
    m_attack_speed : number,
    m_cooldown: number,

    m_transform: TL.Transform,
    m_anim_player: TL.AnimationPlayer,
    m_relationship: TL.Relationship,
    m_dir : TL.Vec2,

    m_control_node_transform: TL.Transform
}

export type WeaponBehavior = typeof(setmetatable({} :: WeaponBehaviorData, WeaponBehavior)) & ScriptBehavior.ScriptBehavior

function WeaponBehavior.new(entity: number): WeaponBehavior
    local self = ScriptBehavior.new(entity)

    self.m_dir = TL.Vec2.ZERO

    return setmetatable(self, WeaponBehavior)
end

function WeaponBehavior.OnInit(self: WeaponBehavior)
    local ctx: TL.GameContext = TL.GetContext()

    local entity = self:GetEntity()
    local transform = ctx:GetTransformManager():Get(entity)
    if transform then
        self.m_control_node_transform = transform
    end

    local relationship = ctx:GetRelationshipManager():Get(entity)

    if relationship:GetChildrenCount() > 0 then
        local weapon_entity = relationship:Get(0)
        self:_initWeapon(weapon_entity)
    else
        TL.Log("weapon controller don't has children")
        return
    end
end

function WeaponBehavior._initWeapon(self: WeaponBehavior, entity: TL.Entity)
    local ctx: TL.GameContext = TL.GetContext()

    local transform = ctx:GetTransformManager():Get(entity)
    if transform then
        self.m_transform = transform
    end

    local anim_player = ctx:GetAnimationPlayerManager():Get(entity)
    if anim_player then
        self.m_anim_player = anim_player
    end

    self.m_dir = TL.Vec2.X_UNIT

    local relationship = ctx:GetRelationshipManager():Get(entity)
    if relationship then
        self.m_relationship = relationship
    end
end

function WeaponBehavior._initFromDefinition(self: WeaponBehavior, definition: TL.WeaponDefinitionHandle)
    self.m_type = definition.m_type
    self.m_image = definition.m_sprite
    self.m_attack_speed = definition.m_attack_speed
    self.m_cooldown = definition.m_cooldown
end

function WeaponBehavior.ChangeDir(self: WeaponBehavior, dir: TL.Vec2)
    self.m_dir = dir
    self.m_control_node_transform.m_rotation = TL.Degrees(TL.GetAngle(self.m_dir, TL.Vec2.Y_UNIT * -1))
end

function WeaponBehavior.GetDir(self: WeaponBehavior): TL.Vec2
    return self.m_dir
end

function WeaponBehavior.Attack(self: WeaponBehavior)
    if not self.m_anim_player:IsPlaying() then
        self.m_anim_player:Stop()
        self.m_anim_player:Play()
    end

    local angle = TL.GetAngle(self.m_dir, TL.Vec2.Y_UNIT * -1)
    self.m_control_node_transform.m_rotation = TL.Degrees(angle)
end

function WeaponBehavior.OnUpdate(self: WeaponBehavior, elapse_time: number)
end

return WeaponBehavior
