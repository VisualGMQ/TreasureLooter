local TL = require("@game/tl_types")
local CharacterBehavior = require("@game/character_behavior")

local Slime = {}
Slime.__index = Slime
setmetatable(Slime, { __index = CharacterBehavior })

export type CharacterDirection = number

type SlimeData = {
	m_sprite: TL.Sprite?,
	m_cct : TL.CharacterController?,
	m_transform : TL.Transform?,
	m_anim_player : TL.AnimationPlayer?,
	m_direction : number, -- CharacterBehavior.Direction

    m_move_cooldown: number,
    m_move_dir: TL.Vec2,

	m_change_move_dir_timer: TL.Timer,
	m_hurt_blinking_timer: TL.Timer,
	m_in_red_state: boolean,
}

export type Slime = typeof(setmetatable({}::SlimeData, Slime)) & CharacterBehavior.CharacterBehavior

function Slime.new(entity: number): Slime
	local self = CharacterBehavior.new(entity)
	return setmetatable(self, Slime)
end

function Slime.OnInit(self: Slime)
	local ctx: TL.GameContext = TL.GetContext()

	local entity = self:GetEntity()

	self.m_direction = CharacterBehavior.Direction.Down
	self.m_sprite = ctx:GetSpriteManager():Get(entity)
	self.m_cct = ctx:GetCCTManager():Get(entity)
	self.m_transform = ctx:GetTransformManager():Get(entity)
	self.m_anim_player = ctx:GetAnimationPlayerManager():Get(entity)

    self.m_move_dir = TL.Vec2.ZERO

	self.m_change_move_dir_timer = ctx:GetTimerManager():Create(3, TL.TimerEventType.Unknown, -1)
	self.m_change_move_dir_timer:Start()

	self.m_hurt_blinking_timer = ctx:GetTimerManager():Create(0.1, TL.TimerEventType.Unknown, 8)
	self.m_in_red_state = false

	self:SubscribeTimerEvent()
	self:SubscribeTimerStopEvent()
end

function Slime.OnUpdate(self: Slime, elapse_time: number)
	self:move(self.m_move_dir, elapse_time)
end

function Slime.move(self: Slime, dir: TL.Vec2, duration: number)
	if dir == TL.Vec2.ZERO and self.m_anim_player ~= nil then
        self.m_anim_player:Stop()
	end

	local old_dir = self.m_direction
	if dir.x < 0 then
		self.m_direction = CharacterBehavior.Direction.Left
	end
	if dir.x > 0 then
		self.m_direction = CharacterBehavior.Direction.Right
	end
	if dir.y < 0 then
		self.m_direction = CharacterBehavior.Direction.Up
	end
	if dir.y > 0 then
		self.m_direction = CharacterBehavior.Direction.Down
	end

	if self.m_cct ~= nil then
		local disp = dir * self.m_move_speed * duration
		self.m_cct:MoveAndSlide(disp)
		self.m_transform.m_position = self.m_cct:GetPosition()
	end

	if (not self.m_anim_player:IsPlaying() and dir ~= TL.Vec2.ZERO) or old_dir ~= self.m_direction then
		if self.m_direction == CharacterBehavior.Direction.Up then
			self.m_anim_player:ChangeAnimation(self.m_move_up_anim)
		elseif self.m_direction == CharacterBehavior.Direction.Left then
			self.m_anim_player:ChangeAnimation(self.m_move_left_anim)
		elseif self.m_direction == CharacterBehavior.Direction.Right then
			self.m_anim_player:ChangeAnimation(self.m_move_right_anim)
		elseif self.m_direction == CharacterBehavior.Direction.Down then
			self.m_anim_player:ChangeAnimation(self.m_move_down_anim)
		end
		self.m_anim_player:Play()
	end

	if dir == TL.Vec2.ZERO then
		if self.m_direction == CharacterBehavior.Direction.Up then
			self.m_sprite.m_region.m_topleft = TL.Vec2(16, 0)
		elseif self.m_direction == CharacterBehavior.Direction.Left then
			self.m_sprite.m_region.m_topleft = TL.Vec2(32, 0)
		elseif self.m_direction == CharacterBehavior.Direction.Right then
			self.m_sprite.m_region.m_topleft = TL.Vec2(48, 0)
		elseif self.m_direction == CharacterBehavior.Direction.Down then
			self.m_sprite.m_region.m_topleft = TL.Vec2(0, 0)
		end
	end
end

function Slime.OnQuit(self: Slime)
	local ctx: TL.GameContext = TL.GetContext()
	if self.m_change_move_dir_timer then
		ctx:GetTimerManager():Remove(self.m_change_move_dir_timer:GetID())
	end
end

function Slime.Hurt(self: Slime, dir: TL.Vec2, damage: number)
	if not self.m_hurt_blinking_timer:IsRunning() then
		self.m_hp = self.m_hp - damage
		self.m_hurt_blinking_timer:Start()
	end

	if self.m_hp <= 0 then
		local ctx: TL.GameContext = TL.GetContext()
		local scene = ctx:GetSceneManager():GetCurrentScene()
		scene:RemoveEntity(self:GetEntity())
	end
end

function Slime.OnTimerEvent(self: Slime, event: TL.TimerEvent)
	local timer_id = event:GetID()
	if timer_id == self.m_change_move_dir_timer:GetID() then
		local x = math.random() - 0.5
		local y = math.random() - 0.5
		self.m_move_dir = TL.Vec2(x, y):Normalize()
	end

	if timer_id == self.m_hurt_blinking_timer:GetID() then
		if self.m_in_red_state then
			self.m_sprite.m_color = TL.Color.White
		else
			self.m_sprite.m_color = TL.Color.Red
		end
		self.m_in_red_state = not self.m_in_red_state
	end
end

function Slime.OnTimerStopEvent(self: Slime, event: Tl.TimerStopEvent)
	local timer_id = event:GetID()
	if timer_id == self.m_hurt_blinking_timer:GetID() then
		self.m_sprite.m_color = TL.Color.White
		self.m_in_red_state = false
	end
end

return Slime
