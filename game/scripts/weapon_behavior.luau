local TL = require("@game/tl_types")
local Config = require("@game/config")
local ScriptBehavior = require("@game/script_behavior")

local WeaponBehavior = {}
WeaponBehavior.__index = WeaponBehavior
setmetatable(WeaponBehavior, { __index = ScriptBehavior })

type WeaponBehaviorData = {
    __config: Config.WeaponConfig,

    m_transform: TL.Transform,
    m_anim_player: TL.AnimationPlayer,
    m_bind_points: TL.BindPoints,
    m_relationship: TL.Relationship,
    m_dir : TL.Vec2,
}

export type WeaponBehavior = typeof(setmetatable({} :: WeaponBehaviorData, WeaponBehavior)) & ScriptBehavior.ScriptBehavior

function WeaponBehavior.new(entity: number): WeaponBehavior
    local self = ScriptBehavior.new(entity)

    self.m_dir = TL.Vec2.ZERO

    return setmetatable(self, WeaponBehavior)
end

function WeaponBehavior.OnInit(self: WeaponBehavior)
    local ctx: TL.GameContext = TL.GetContext()

    local transform = ctx:GetTransformManager():Get(self:GetEntity())
    if transform then
        self.m_transform = transform
    end

    local anim_player = ctx:GetAnimationPlayerManager():Get(self:GetEntity())
    if anim_player then
        self.m_anim_player = anim_player
    end

    self.m_dir = TL.Vec2.X_UNIT

    local bind_point_component = ctx:GetBindPointsComponentManager():Get(self:GetEntity())
    if bind_point_component then
        self.m_bind_points = bind_point_component
    end

    local relationship = ctx:GetRelationshipManager():Get(self:GetEntity())
    if relationship then
        self.m_relationship = relationship
    end
end

function WeaponBehavior.__attach_config(self: WeaponBehavior, config: Config.WeaponConfig)
    self.__config = config
end

function WeaponBehavior.GetWeaponConfig(self: WeaponBehavior): Config.WeaponConfig
    return self.__config
end

function WeaponBehavior.ChangeDir(self: WeaponBehavior, dir: TL.Vec2)
    self.m_dir = dir
    self.m_transform.m_rotation = TL.Degrees(TL.GetAngle(self.m_dir, TL.Vec2.Y_UNIT * -1))
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
    self.m_transform.m_rotation = TL.Degrees(angle);
end

function WeaponBehavior.OnUpdate(self: WeaponBehavior, elapse_time: number)
    local ctx: TL.GameContext = TL.GetContext()

    for i, entity in ipairs(self.m_relationship.m_children) do
        local transform = ctx:GetTransformManager():Get(entity)
        local trigger = ctx:GetTriggerComponentManager():Get(entity)

        if not transform or not trigger then
            continue
        end

        local bind_point_name = trigger:GetBindPointName()
        if bind_point_name == "" then
            continue
        end

        local bind_point: TL.BindPoint = self.m_bind_points.m_bind_points[bind_point_name]
        if not bind_point then
            continue
        end

        transform.m_position = bind_point.m_position
    end
end

return WeaponBehavior
