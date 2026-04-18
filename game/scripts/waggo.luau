local TL = require("@game/tl_types")
local CharacterBehavior = require("@game/character_behavior")
local Slime = require("@game/slime")

local Waggo = {}
Waggo.__index = Waggo
setmetatable(Waggo, { __index = CharacterBehavior })

type WaggoData = {
	m_sprite: TL.Sprite?,
	m_cct : TL.CharacterController?,
	m_transform : TL.Transform?,
	m_anim_player : TL.AnimationPlayer?,
	m_direction : CharacterBehavior.Direction,
}

export type Waggo = typeof(setmetatable({}::WaggoData, Waggo)) & CharacterBehavior.CharacterBehavior

function Waggo.new(entity: number): Waggo
	local self = CharacterBehavior.new(entity)
	return setmetatable(self, Waggo)
end

function Waggo.OnInit(self: Waggo)
	local ctx: TL.GameContext = TL.GetContext()

	local entity = self:GetEntity()

	self.m_direction = CharacterBehavior.Direction.Down
	self.m_sprite = ctx:GetSpriteManager():Get(entity)
	self.m_cct = ctx:GetCCTManager():Get(entity)
	self.m_transform = ctx:GetTransformManager():Get(entity)
	self.m_anim_player = ctx:GetAnimationPlayerManager():Get(entity)

	self:SubscribeDebugEvent()
	self:SubscribeTriggerEnterEvent()
end

function Waggo.OnUpdate(self: Waggo, elapse_time: number)
	local ctx = TL.GetContext()
	local input_manager = ctx:GetInputManager()
	local action = input_manager:GetAction("Attack")
	local axises = input_manager:MakeAxises("MoveX", "MoveY"):Value(0)
	if (action:IsPressed(0)) then
		self:attack()
	end
	self:move(axises, elapse_time)
	ctx:GetCamera():MoveTo(self.m_transform.m_position)
end

function Waggo.attack(self: Waggo)
	self.m_weapon:Attack()
end

function Waggo.move(self: Waggo, dir, duration: number)
	if dir == TL.Vec2.ZERO then
		if self.m_anim_player ~= nil then
			self.m_anim_player:Stop()
		end
	elseif self.m_weapon and not self.m_weapon.m_anim_player:IsPlaying() then
		self.m_weapon:ChangeDir(dir:Normalize())
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

function Waggo.OnQuit(self: Waggo)
end

function Waggo.OnRender(self: Waggo)
end

function Waggo.OnDebugEvent(self: Waggo, event: TL.DebugEvent)
	TL.Log("debug event here")
end

function Waggo.OnTriggerEnterEvent(self: Waggo, event: TL.TriggerEnterEvent)
	local dst_entity = event:GetOverlapResult().m_dst_entity
	if dst_entity == TL.null_entity or dst_entity == self:GetEntity() then
		return
	end

	local ctx: TL.GameContext = TL.GetContext()
	local script_mgr = ctx:GetScriptManager()
	local dst_script = script_mgr:Get(dst_entity) :: Slime.Slime

	dst_script:Hurt(self.m_weapon:GetDir(), 1)
end


return Waggo
