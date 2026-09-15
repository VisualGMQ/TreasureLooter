local MoveComponent = require("common.components.move")
local Common = require("common.common")

---@class ClientMoveComponentDefinition
---@field m_cct CharacterController|nil
---@field m_anim AnimationPlayer
---@field m_speed number
---@field m_move_up_anim AnimationHandle
---@field m_move_down_anim AnimationHandle
---@field m_move_left_anim AnimationHandle
---@field m_move_right_anim AnimationHandle

---@class ClientMoveComponent : MoveComponent
---@field _direction CommonDirection
---@field _anim_player AnimationPlayer
---@field _sprite Sprite
---@field _move_up_anim AnimationHandle
---@field _move_down_anim AnimationHandle
---@field _move_left_anim AnimationHandle
---@field _move_right_anim AnimationHandle
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = MoveComponent })

---@param gameobject any
---@param definition ClientMoveComponentDefinition
---@return ClientMoveComponent
function _M.new(gameobject, definition)
    local self = MoveComponent.new(gameobject, definition.m_cct, definition.m_speed)
    ---@cast self ClientMoveComponent
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

---@param elapse_time TimeType
function _M:Update(elapse_time)
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
