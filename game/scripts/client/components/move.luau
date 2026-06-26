local MoveComponent = require("@common/components/move")
local Common = require("@common/common")
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = MoveComponent })

type ClientMoveComponentData = {
    _direction:Common.Direction,
    _anim_player:AnimationPlayer,
    _sprite:Sprite,

    _move_up_anim:AnimationHandle,
    _move_down_anim:AnimationHandle,
    _move_left_anim:AnimationHandle,
    _move_right_anim:AnimationHandle,
}
export type ClientMoveComponent = typeof(_M) & ClientMoveComponentData & MoveComponent.MoveComponent

export type ClientMoveComponentDefinition = {
    m_cct:CharacterController?,
	m_anim:AnimationPlayer,
	m_speed:number,
    m_move_up_anim:AnimationHandle,
    m_move_down_anim:AnimationHandle,
    m_move_left_anim:AnimationHandle,
    m_move_right_anim:AnimationHandle,
}

function _M.new(gameobject, definition: ClientMoveComponentDefinition):ClientMoveComponent
    local self = MoveComponent.new(gameobject, definition.m_cct, definition.m_speed) :: ClientMoveComponent
    local ctx = TL_Client.GetContext()
    local entity = gameobject:GetEntity()
    self._direction = Common.Direction.Down
	self._anim_player = definition.m_anim
    self._move_up_anim = definition.m_move_up_anim
    self._move_down_anim = definition.m_move_down_anim
    self._move_left_anim = definition.m_move_left_anim
    self._move_right_anim = definition.m_move_right_anim
    self._sprite = ctx:GetSpriteManager():Get(entity)
    return setmetatable(self, _M)
end

function _M.Update(self:ClientMoveComponent, elapse_time:TimeType)
    MoveComponent.Update(self, elapse_time)

    if not self:IsWantMoving() then
		if self._anim_player ~= nil then
			self._anim_player:Stop()
		end
		if self._direction == Common.Direction.Up then
			self._sprite.m_region.m_topleft = TL_Common.Vec2(16, 0)
		elseif self._direction == Common.Direction.Left then
			self._sprite.m_region.m_topleft = TL_Common.Vec2(32, 0)
		elseif self._direction == Common.Direction.Right then
			self._sprite.m_region.m_topleft = TL_Common.Vec2(48, 0)
		elseif self._direction == Common.Direction.Down then
			self._sprite.m_region.m_topleft = TL_Common.Vec2(0, 0)
		end
		return
    end

    local old_direction = self._direction
    local velocity = self:GetVelocity()
	if math.abs(velocity.x) > math.abs(velocity.y) then
		if velocity.x < 0 then
			self._direction = Common.Direction.Left
		else
			self._direction = Common.Direction.Right
		end
	else
		if velocity.y < 0 then
			self._direction = Common.Direction.Up
		else
			self._direction = Common.Direction.Down
		end
	end

	if (not self._anim_player:IsPlaying() and velocity ~= TL_Common.Vec2.ZERO) or old_direction ~= self._direction then
		if self._direction == Common.Direction.Up then
			self._anim_player:ChangeAnimation(self._move_up_anim)
		elseif self._direction == Common.Direction.Left then
			self._anim_player:ChangeAnimation(self._move_left_anim)
		elseif self._direction == Common.Direction.Right then
			self._anim_player:ChangeAnimation(self._move_right_anim)
		elseif self._direction == Common.Direction.Down then
			self._anim_player:ChangeAnimation(self._move_down_anim)
		end
		self._anim_player:Play()
	end
end

return _M