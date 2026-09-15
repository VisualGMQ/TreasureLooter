local Component = require("common.components.component")

---@class MoveComponent : Component
---@field m_cct CharacterController|nil
---@field _move_velocity Vec2
---@field _speed number
---@field _move_dir Vec2
---@field _last_move_dir Vec2
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Component })

---@param gameobject any
---@param cct CharacterController|nil
---@param speed number
---@return MoveComponent
function _M.new(gameobject, cct, speed)
    local self = Component.new(gameobject)
    ---@cast self MoveComponent
    self._move_velocity = TL_Common.Vec2.ZERO
    self.m_cct = cct
    self._speed = speed
    self._move_dir = TL_Common.Vec2.ZERO
    self._last_move_dir = TL_Common.Vec2.ZERO
    return setmetatable(self, _M)
end

---@return Vec2
function _M:GetMoveDirection()
    return self._move_dir
end

--- Last direction the object actually moved towards. Keeps the previous
--- facing when the object stops, unlike GetMoveDirection which is the raw
--- current input direction.
---@return Vec2
function _M:GetLastMoveDirection()
    return self._last_move_dir
end

---@param speed number
function _M:ChangeSpeed(speed)
    self._speed = speed
    self._move_velocity = self._move_dir * speed
end

---@return number
function _M:GetSpeed()
    return self._speed
end

---@param dir Vec2
function _M:SetDir(dir)
    self._move_dir = dir
    self._move_velocity = dir * self._speed
end

---@return Vec2
function _M:GetVelocity()
    return self._move_velocity
end

---@param position Vec2
function _M:Teleport(position)
    if self.m_cct then
        self.m_cct:Teleport(position)
        local new_position = self.m_cct:GetPosition()
        self:GetGameObject().m_transform.m_position = new_position
    else
        self:GetGameObject().m_transform.m_position = position
    end
end

function _M:StopMove()
    self._move_velocity = TL_Common.Vec2.ZERO
end

---@return boolean
function _M:IsWantMoving()
    return self._move_velocity:LengthSquared() > 0
end

---@param elapse_time TimeType
function _M:Update(elapse_time)
    local disp = self._move_velocity * elapse_time
    if disp:LengthSquared() == 0 then
        return
    end

    self._last_move_dir = self._move_velocity:Normalize()

    local transform = self:GetGameObject().m_transform
    if self.m_cct then
        self.m_cct:MoveAndSlide(disp)
        local new_position = self.m_cct:GetPosition()
        transform.m_position = new_position
    else
        transform.m_position = transform.m_position + disp
    end
end

return _M
