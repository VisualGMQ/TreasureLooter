local MoveComponent = require("common.components.move")
local Common = require("common.common")

---@class ClientMoveComponentDefinition
---@field m_cct CharacterController|nil
---@field m_anim AnimationPlayer
--- The speed of the character definition is unused now: the component has its
--- own m_max_speed (see MoveComponent).
---@field m_speed number
---@field m_move_up_anim AnimationHandle
---@field m_move_down_anim AnimationHandle
---@field m_move_left_anim AnimationHandle
---@field m_move_right_anim AnimationHandle

---@class ClientMoveComponent : MoveComponent
---@field private _direction Direction
---@field private _move_dir Vec2
---@field private _server_velocity Vec2
---@field private _anim_player AnimationPlayer
---@field private _sprite Sprite?
---@field private _move_up_anim AnimationHandle
---@field private _move_down_anim AnimationHandle
---@field private _move_left_anim AnimationHandle
---@field private _move_right_anim AnimationHandle
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = MoveComponent })

---@param gameobject any
---@param definition ClientMoveComponentDefinition
---@return ClientMoveComponent
function _M.new(gameobject, definition)
    local self = MoveComponent.new(gameobject, definition.m_cct)
    ---@cast self ClientMoveComponent
    local ctx = TL_Client.GetContext()
    local entity = gameobject:GetEntity()
    self._direction = Common.Direction.Down
    self._anim_player = definition.m_anim
    self._move_up_anim = definition.m_move_up_anim
    self._move_down_anim = definition.m_move_down_anim
    self._move_left_anim = definition.m_move_left_anim
    self._move_right_anim = definition.m_move_right_anim
    self._move_dir = TL_Common.Vec2.ZERO
    self._server_velocity = TL_Common.Vec2.ZERO
    self._sprite = ctx:GetSpriteManager():Get(entity)
    return setmetatable(self, _M)
end

--- The velocity the server applies on top of the input (a collision push),
--- mirrored locally so the prediction doesn't lag behind the authoritative
--- position.
---@param velocity Vec2
function _M:SetServerVelocity(velocity)
    self._server_velocity = velocity
end

---@return Vec2
function _M:GetMoveDirection()
    return self._move_dir
end

---@param speed number
function _M:ChangeSpeed(speed)
    self:SetMaxSpeed(speed)
end

---@return number
function _M:GetSpeed()
    return self:GetMaxSpeed()
end

--- The direction the input asks for; it only drives the facing and the
--- animation, the movement itself comes from `Accelerate` and `m_velocity`.
---@param dir Vec2
function _M:SetDir(dir)
    self._move_dir = dir
end

--- Whether there is input this frame, so the walk animation still follows the
--- keys even while the character is sliding from a collision.
---@return boolean
function _M:IsWantMoving()
    return self._move_dir:LengthSquared() > 0
end

--- The server owns the collision response, so the client never pushes itself:
--- it predicts its own input and follows the authoritative position instead.
---@return boolean
function _M:IsCollisionImpulseAuthority()
    return false
end

---@param elapse_time TimeType?
function _M:Update(elapse_time)
    if elapse_time and self._server_velocity:LengthSquared() > 0 then
        self:AddMoveDisp(self._server_velocity * elapse_time)
    end

    MoveComponent.Update(self, elapse_time)

    if not self:IsWantMoving() then
        if elapse_time then
            -- Drop the accumulated input force instead of keeping it as
            -- momentum: otherwise releasing one direction and pressing another
            -- shortly after bends the movement into an arc.
            self:SetVelocity(TL_Common.Vec2.ZERO)
        end
        if self._anim_player ~= nil then
            self._anim_player:Stop()
        end
        if self._sprite then
            if self._direction == Common.Direction.Up then
                self._sprite.m_region.m_topleft = TL_Common.Vec2(16, 0)
            elseif self._direction == Common.Direction.Left then
                self._sprite.m_region.m_topleft = TL_Common.Vec2(32, 0)
            elseif self._direction == Common.Direction.Right then
                self._sprite.m_region.m_topleft = TL_Common.Vec2(48, 0)
            elseif self._direction == Common.Direction.Down then
                self._sprite.m_region.m_topleft = TL_Common.Vec2(0, 0)
            end
        end
        return
    end

    -- The direction comes from the requested direction and not from the
    -- velocity: a replicated character has no velocity of its own, it only
    -- gets the direction of the snapshots, and reading the velocity here used
    -- to leave it facing down and without any walk animation.
    local old_direction = self._direction
    local move_dir = self._move_dir
    if math.abs(move_dir.x) > math.abs(move_dir.y) then
        if move_dir.x < 0 then
            self._direction = Common.Direction.Left
        else
            self._direction = Common.Direction.Right
        end
    else
        if move_dir.y < 0 then
            self._direction = Common.Direction.Up
        else
            self._direction = Common.Direction.Down
        end
    end

    if not self._anim_player:IsPlaying() or old_direction ~= self._direction then
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
