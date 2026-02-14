local Waggo = {}
Waggo.__index = Waggo

type WaggoData = {
	m_entity: number,
	m_sprite: userdata,
}

export type State = number

local CharacterDirection = {
    Left = 0,
    Right = 1,
    Down = 2,
    Up = 3,
} :: { [string]: State }


export type Waggo = typeof(setmetatable({}::WaggoData, Waggo))

function Waggo:new(entity: number): Waggo
	local self = {}
	print(entity)
	self.m_sprite = nil
	self.m_anim_player = nil
	self.m_cct = nil
	self.m_transform = nil
	self.m_entity = entity
	self.m_image_sheet = nil
	self.m_move_left_anim = nil
	self.m_move_right_anim = nil
	self.m_move_up_anim = nil
	self.m_move_down_anim = nil
	self.m_direction = CharacterDirection.Down
	self.m_move_speed = 100

	return setmetatable(self, Waggo)
end

function Waggo:OnInit()
	local ctx = TL.GetContext()
	self.m_sprite = ctx:GetSpriteManager():Get(self.m_entity)
	self.m_cct = ctx:GetCCTManager():Get(self.m_entity)
	self.m_transform = ctx:GetTransformManager():Get(self.m_entity)
	self.m_anim_player = ctx:GetAnimationPlayerManager():Get(self.m_entity)

	local config = ctx:GetGameplayConfigManager():Get(self.m_entity)
	self.m_image_sheet = config.m_sprite_sheet
	self.m_move_left_anim = config.m_move_left_animation
	self.m_move_right_anim = config.m_move_right_animation
	self.m_move_up_anim = config.m_move_up_animation
	self.m_move_down_anim = config.m_move_down_animation
	self.m_move_speed = config.m_speed

	local relationship = ctx:GetRelationshipManager():Get(self.m_entity)
	--[[
	local children = relationship.m_children
	std::vector<T>& not register yet
	--]]
end

function Waggo:OnUpdate(elapse_time: number)
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

function Waggo:attack()
end

function Waggo:move(dir, duration: number)
	if dir == TL.Vec2.ZERO then
		if self.m_anim_player ~= nil then
			self.m_anim_player:Stop()
		end
	else
		-- do weapon relate
		--[[
		if (!m_weapon_script.m_anim_player.IsPlaying()) {
			m_weapon_script.ChangeDir(dir.Normalize());
		}
		--]]
	end

	local old_dir = self.m_direction
	if dir.x < 0 then
		self.m_direction = CharacterDirection.Left
	end
	if dir.x > 0 then
		self.m_direction = CharacterDirection.Right
	end
	if dir.y < 0 then
		self.m_direction = CharacterDirection.Up
	end
	if dir.y > 0 then
		self.m_direction = CharacterDirection.Down
	end

	if self.m_cct ~= nil then
		local disp = dir * self.m_move_speed * duration
		self.m_cct:MoveAndSlide(disp)
		self.m_transform.m_position = self.m_cct:GetPosition()
	end

	if (not self.m_anim_player:IsPlaying() and dir ~= TL.Vec2.ZERO) or old_dir ~= self.m_direction then
		if self.m_direction == CharacterDirection.Up then
			self.m_anim_player:ChangeAnimation(self.m_move_up_anim)
		elseif self.m_direction == CharacterDirection.Left then
			self.m_anim_player:ChangeAnimation(self.m_move_left_anim)
		elseif self.m_direction == CharacterDirection.Right then
			self.m_anim_player:ChangeAnimation(self.m_move_right_anim)
		elseif self.m_direction == CharacterDirection.Down then
			self.m_anim_player:ChangeAnimation(self.m_move_down_anim)
		end
		self.m_anim_player:Play()
	end

	if dir == TL.Vec2.ZERO then
		if self.m_direction == CharacterDirection.Up then
			self.m_sprite.m_region.m_topleft = TL.Vec2(16, 0)
		elseif self.m_direction == CharacterDirection.Left then
			self.m_sprite.m_region.m_topleft = TL.Vec2(32, 0)
		elseif self.m_direction == CharacterDirection.Right then
			self.m_sprite.m_region.m_topleft = TL.Vec2(48, 0)
		elseif self.m_direction == CharacterDirection.Down then
			self.m_sprite.m_region.m_topleft = TL.Vec2(0, 0)
		end
	end

	self.m_sprite.m_z_order = TL.GetZOrderByYSorting(
		self.m_transform.m_position.y + self.m_sprite.m_size.y * 0.5,
		TL.RenderLayer.TilemapArch)
end

function Waggo:OnQuit()
	print("OnQuit")
end

function Waggo:OnRender()
end

return Waggo