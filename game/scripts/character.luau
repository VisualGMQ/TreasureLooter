local TL = require("@game/tl_types")
local Weapon = require("@game/weapon")
local Common = require("@game/common")
local ScriptBehavior = require("@game/script_behavior")

local Character = {}
Character.__index = Character

type CharacterData = {
    m_weapon : Weapon.Weapon?,

    -- from definition
    m_hp: number,
    m_move_left_anim : TL.AnimationHandle,
    m_move_right_anim : TL.AnimationHandle,
    m_move_up_anim : TL.AnimationHandle,
    m_move_down_anim : TL.AnimationHandle,
    m_move_speed : number,
    m_interactable_detect_trigger : TL.Trigger?,
	m_sprite: TL.Sprite,
	m_cct : TL.CharacterController?,
	m_transform : TL.Transform,
	m_anim_player : TL.AnimationPlayer,

    -- from runtime
    m_team_id: TL.TeamID,
	m_direction : Common.Direction,

    -- hurt relate
	m_change_move_dir_timer: TL.Timer,
	m_hurt_blinking_timer: TL.Timer,
	m_in_red_state: boolean,
}

export type Character = typeof(setmetatable({} :: CharacterData, Character))

function Character.new(entity: TL.Entity, definition: TL.CharacterDefinitionHandle, weapon: Weapon.Weapon?): Character
    local self = {} :: CharacterData
    self.m_weapon = weapon

    self.m_hp = definition.m_hp
    self.m_move_left_anim = definition.m_move_left_animation
    self.m_move_right_anim = definition.m_move_right_animation
    self.m_move_up_anim = definition.m_move_up_animation
    self.m_move_down_anim = definition.m_move_down_animation
    self.m_move_speed = definition.m_speed
    self.m_team_id = TL.TeamID.Team_invalid

    local ctx: TL.GameContext = TL.GetContext()
    local trigger_manager = ctx:GetTriggerComponentManager()
    self.m_interactable_detect_trigger = trigger_manager:Get(entity)

	self.m_sprite = ctx:GetSpriteManager():Get(entity)
	self.m_cct = ctx:GetCCTManager():Get(entity)
	self.m_transform = ctx:GetTransformManager():Get(entity)
	self.m_anim_player = ctx:GetAnimationPlayerManager():Get(entity)

    self.m_direction = Common.Direction.Down

	self.m_hurt_blinking_timer = ctx:GetTimerManager():Create(0.1, TL.TimerEventType.Unknown, 8)
	self.m_in_red_state = false

    return setmetatable(self, Character)
end

function Character.SubscribeEvents(self:Character,  behavior: ScriptBehavior.ScriptBehavior)
	behavior:SubscribeDebugEvent()
	behavior:SubscribeTriggerEnterEvent()
	behavior:SubscribeTimerEvent()
	behavior:SubscribeTimerStopEvent()
end

function Character.SetTeamID(self: Character, team_id: TL.TeamID)
    self.m_team_id = team_id
end

function Character.GetTeamID(self: Character): TL.TeamID
    return self.m_team_id
end

function Character.GetWeaponEntity(self: Character): TL.Entity
    return self._weapon_entity
end

function Character.GetWeaponControlNode(self: Character): TL.Entity
    return self._weapon_control_node
end

function Character.Attack(self:Character)
    if self.m_weapon then
        self.m_weapon:Attack()
    end
end

function Character.Move(self:Character, dir: TL.Vec2, duration: TL.TimeType)
    if dir == TL.Vec2.ZERO then
		if self.m_anim_player ~= nil then
			self.m_anim_player:Stop()
		end
	elseif self.m_weapon and not self.m_weapon.m_anim_player:IsPlaying() then
		self.m_weapon:ChangeDir(dir:Normalize())
	end

	local old_dir = self.m_direction
	if dir.x < 0 then
		self.m_direction = Common.Direction.Left
	end
	if dir.x > 0 then
		self.m_direction = Common.Direction.Right
	end
	if dir.y < 0 then
		self.m_direction = Common.Direction.Up
	end
	if dir.y > 0 then
		self.m_direction = Common.Direction.Down
	end

	if self.m_cct ~= nil then
		local disp = dir * self.m_move_speed * duration
		self.m_cct:MoveAndSlide(disp)
		self.m_transform.m_position = self.m_cct:GetPosition()
	end

	if (not self.m_anim_player:IsPlaying() and dir ~= TL.Vec2.ZERO) or old_dir ~= self.m_direction then
		if self.m_direction == Common.Direction.Up then
			self.m_anim_player:ChangeAnimation(self.m_move_up_anim)
		elseif self.m_direction == Common.Direction.Left then
			self.m_anim_player:ChangeAnimation(self.m_move_left_anim)
		elseif self.m_direction == Common.Direction.Right then
			self.m_anim_player:ChangeAnimation(self.m_move_right_anim)
		elseif self.m_direction == Common.Direction.Down then
			self.m_anim_player:ChangeAnimation(self.m_move_down_anim)
		end
		self.m_anim_player:Play()
	end

	if dir == TL.Vec2.ZERO then
		if self.m_direction == Common.Direction.Up then
			self.m_sprite.m_region.m_topleft = TL.Vec2(16, 0)
		elseif self.m_direction == Common.Direction.Left then
			self.m_sprite.m_region.m_topleft = TL.Vec2(32, 0)
		elseif self.m_direction == Common.Direction.Right then
			self.m_sprite.m_region.m_topleft = TL.Vec2(48, 0)
		elseif self.m_direction == Common.Direction.Down then
			self.m_sprite.m_region.m_topleft = TL.Vec2(0, 0)
		end
	end

end

function Character.Hurt(self: Character, entity: TL.Entity, damage: number)
	if not self.m_hurt_blinking_timer:IsRunning() then
		self.m_hp = self.m_hp - damage
		self.m_hurt_blinking_timer:Start()
	end

	if self.m_hp <= 0 then
		local ctx: TL.GameContext = TL.GetContext()
		local scene = ctx:GetSceneManager():GetCurrentScene()
		scene:RemoveEntity(entity)
	end
end

function Character.OnTimerEvent(self: Character, event: TL.TimerEvent)
	local timer_id = event:GetID()

	if timer_id == self.m_hurt_blinking_timer:GetID() then
		if self.m_in_red_state then
			self.m_sprite.m_color = TL.Color.White
		else
			self.m_sprite.m_color = TL.Color.Red
		end
		self.m_in_red_state = not self.m_in_red_state
	end
end

function Character.OnTimerStopEvent(self: Character, event: TL.TimerStopEvent)
	local timer_id = event:GetID()
	if timer_id == self.m_hurt_blinking_timer:GetID() then
		self.m_sprite.m_color = TL.Color.White
		self.m_in_red_state = false
	end
end

return Character
