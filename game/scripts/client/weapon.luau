local TL = require("@client/tl_types")
local TLC = TL.Common
local TLI = TL.Client
local TLS = TL.Server
local TLSch = TL.Schema

local ItemBehavior = require("@client/item_behavior")

local Weapon = {}
Weapon.__index = Weapon

--[[
Weapon node tree:

---- weapon controler (for play attack animation, WeaponBehavior's entity)
  |---- weapon sprite & collision
--]]

type WeaponData = {
    m_type : TLSch.WeaponType,
    m_image : TLC.ImageHandle,
    m_attack_speed : number,
    m_cooldown: number,
    m_damage : number,

    m_weapon_entity: TLC.Entity,
    m_transform: TLC.Transform,
    m_anim_player: TLI.AnimationPlayer,
    m_relationship: TLC.Relationship,
    m_dir : TLC.Vec2,

    m_control_node_transform: TLC.Transform,

    m_digging_timer: TLC.Timer,
    m_digging_entity: TLC.Entity,
}

export type Weapon = typeof(setmetatable({} :: WeaponData, Weapon))

function Weapon.new(entity: TLC.Entity, definition: TLSch.WeaponDefinitionHandle): Weapon?
    local self = {} :: WeaponData
    self = setmetatable(self, Weapon)

    self.m_dir = TLC.Vec2.ZERO

    local ctx: TLI.ClientContext = TLI.GetClientContext()

    local transform = ctx:GetTransformManager():Get(entity)
    if transform then
        self.m_control_node_transform = transform
    end

    local relationship = ctx:GetRelationshipManager():Get(entity)

    if relationship:GetChildrenCount() > 0 then
        self.m_weapon_entity = relationship:Get(0)
        self:_initWeapon(self.m_weapon_entity)
    else
        TLC.Log("weapon controller don't has children")
        return nil
    end

    if definition:IsValid() then
        self.m_type = definition.m_type
        self.m_image = definition.m_sprite
        self.m_attack_speed = definition.m_attack_speed
        self.m_cooldown = definition.m_cooldown
        self.m_damage = definition.m_damage
    end

    -- trivial interval, will change by digging target
    self.m_digging_timer = ctx:GetTimerManager():Create(1, TLC.TimerEventType.Digging, 1)
    self.m_digging_entity = TLC.null_entity

    return self
end

function Weapon.OnQuit(self: Weapon)
	local ctx: TLI.ClientContext = TLI.GetClientContext()
	if self.m_digging_timer then
		ctx:GetTimerManager():Remove(self.m_digging_timer:GetID())
	end
end

function Weapon._initWeapon(self: Weapon, entity: TLC.Entity)
    local ctx: TLI.ClientContext = TLI.GetClientContext()

    local transform = ctx:GetTransformManager():Get(entity)
    if transform then
        self.m_transform = transform
    end

    local anim_player = ctx:GetAnimationPlayerManager():Get(entity)
    if anim_player then
        self.m_anim_player = anim_player
    end

    self.m_dir = TLC.Vec2.X_UNIT

    local relationship = ctx:GetRelationshipManager():Get(entity)
    if relationship then
        self.m_relationship = relationship
    end

    ctx:GetTriggerComponentManager():Disable(self.m_weapon_entity)
end

function Weapon.ChangeDir(self: Weapon, dir: TLC.Vec2)
    self.m_dir = dir
    self.m_control_node_transform.m_rotation = TLC.Degrees(TLC.GetAngle(TLC.Vec2.Y_UNIT * -1, self.m_dir))
end

function Weapon.GetDir(self: Weapon): TLC.Vec2
    return self.m_dir
end

function Weapon.Attack(self: Weapon)
    local ctx: TLI.ClientContext = TLI.GetClientContext()

    ctx:GetTriggerComponentManager():Enable(self.m_weapon_entity)

    if not self.m_anim_player:IsPlaying() then
        self.m_anim_player:Stop()
        self.m_anim_player:Play()
    end

    local angle = TLC.GetAngle(TLC.Vec2.Y_UNIT * -1, self.m_dir)
    self.m_control_node_transform.m_rotation = TLC.Degrees(angle)
end

function Weapon.OnUpdate(self: Weapon, elapse_time: number)
    local ctx: TLI.ClientContext = TLI.GetClientContext()

    -- FIXME: set state every frame may cause performance down
    if not self.m_anim_player:IsPlaying() then
        ctx:GetTriggerComponentManager():Disable(self.m_weapon_entity)
    end
end

function Weapon.OnTriggerEnterEvent(self: Weapon, event: TLC.TriggerEnterEvent)
	local dst_entity = event:GetOverlapResult().m_dst_entity
	if dst_entity == TLC.null_entity or dst_entity == self.m_weapon_entity then
		return
	end

	local ctx: TLI.ClientContext = TLI.GetClientContext()
	local script_mgr = ctx:GetScriptManager()
	local script = script_mgr:Get(dst_entity)

    if not script then
        return
    end

    local event_type = event:GetType()
    if event_type == TLC.TriggerEventType.WeaponAttack and script.m_character then
        script.m_character:Hurt(self.m_damage)
        return
    end

    if event_type == TLC.TriggerEventType.Digging and script.m_item
        and script.m_item.m_behavior:Has(TLSch.ItemBehavior.Diggable) then

        if self.m_digging_entity ~= dst_entity then
            -- check changed target
            self.m_digging_timer:SetInterval(script.m_item.m_dig_time)
            self.m_digging_timer:Start()
            TLC.Log("do digging")
        end
        -- TODO: do digging
    end
end

function Weapon.OnTimerStopEvent(self: Weapon, event: TLC.TimerStopEvent)
    if event:GetEventType() == TLC.TimerEventType.Digging and self.m_digging_entity ~= TLC.null_entity then
        -- TODO: spawn mineral fragment
        local ctx: TLI.ClientContext = TLI.GetClientContext()
        local scene = ctx:GetSceneManager():GetCurrentScene()

        local item_behavior = ctx:GetScriptManager():Get(self.m_digging_entity) :: ItemBehavior.ItemBehavior

        local fragment_entity, fragment_script = Creation.CreateItem(item_behavior.m_item.m_mineral_fragment)

        -- TODO: put on player layer
        local root_relationship = ctx:GetRelationshipManager():Get(scene:GetRootEntity())
        root_relationship:AddChild(fragment_entity)
    end
end

return Weapon


