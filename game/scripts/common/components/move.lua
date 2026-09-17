local Component = require("common.components.component")

local k_default_acceleration = 12000
local k_default_collision_impulse = 3000
local k_default_friction = 800
local k_default_mass = 10
local k_default_max_speed = 300
-- Time before the same character can be pushed again; without it both get
-- zeroed and re-impulsed every frame while they are in contact, which makes
-- the two characters jitter.
local k_collision_interval = 0.25

---@class MoveComponent : Component
---@field m_cct CharacterController|nil
---@field m_mass number
---@field m_max_speed number
---@field m_velocity Vec2
---@field m_acceleration number
---@field m_friction number
---@field m_collision_impulse number
---@field m_collision_interval TimeType
---@field private _move_disp Vec2
---@field private _last_move_dir Vec2
---@field private _collision_timer TimeType
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Component })

---@param gameobject any
---@param cct CharacterController|nil
---@return MoveComponent
function _M.new(gameobject, cct)
    local self = Component.new(gameobject)
    ---@cast self MoveComponent
    self._move_disp = TL_Common.Vec2.ZERO
    self._last_move_dir = TL_Common.Vec2.ZERO
    self._collision_timer = 0
    self.m_cct = cct

    self.m_mass = k_default_mass
    self.m_max_speed = k_default_max_speed
    self.m_velocity = TL_Common.Vec2.ZERO
    -- The input force of one second; the character accelerates by
    -- m_acceleration / m_mass per second.
    self.m_acceleration = k_default_acceleration
    -- Deceleration applied when there is no input.
    self.m_friction = k_default_friction
    -- Constant impulse given to both characters on collision, so a heavier
    -- character is pushed away slower.
    self.m_collision_impulse = k_default_collision_impulse
    -- Minimum time between two pushes on the same character.
    self.m_collision_interval = k_collision_interval
    return setmetatable(self, _M)
end

---@return number
function _M:GetMass()
    return self.m_mass
end

---@param mass number
function _M:SetMass(mass)
    self.m_mass = mass
end

---@return number
function _M:GetMaxSpeed()
    return self.m_max_speed
end

---@param speed number
function _M:SetMaxSpeed(speed)
    self.m_max_speed = speed
    self:SetVelocity(self.m_velocity)
end

---@return Vec2
function _M:GetVelocity()
    return self.m_velocity
end

---@param velocity Vec2
function _M:SetVelocity(velocity)
    local speed = velocity:Length()
    if speed > self.m_max_speed and speed > 0 then
        velocity = velocity * (self.m_max_speed / speed)
    end
    self.m_velocity = velocity
end

---@param impulse Vec2
function _M:AddImpulse(impulse)
    self:SetVelocity(self.m_velocity + impulse / self.m_mass)
end

--- Add the input force of one frame as an impulse towards `dir`.
---@param dir Vec2
---@param elapse_time TimeType
function _M:Accelerate(dir, elapse_time)
    local length_squared = dir:LengthSquared()
    if length_squared == 0 or elapse_time <= 0 then
        return
    end

    self:AddImpulse(dir * (self.m_acceleration * elapse_time / math.sqrt(length_squared)))
end

--- Decelerate to zero, used while there is no input.
---@param elapse_time TimeType
function _M:ApplyFriction(elapse_time)
    local speed = self.m_velocity:Length()
    if speed <= 0 or elapse_time <= 0 then
        return
    end

    local rest = math.max(0, speed - self.m_friction * elapse_time)
    self.m_velocity = self.m_velocity * (rest / speed)
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
    self._move_disp = TL_Common.Vec2.ZERO
end

---@return boolean
function _M:IsWantMoving()
    return self._move_disp:LengthSquared() > 0
end

---@return Vec2
function _M:GetMoveDisp()
    return self._move_disp
end

--- Last direction the object actually moved towards. Keeps the previous facing
--- when the object stops, unlike the raw input direction.
---@return Vec2
function _M:GetLastMoveDirection()
    return self._last_move_dir
end

---@param disp Vec2
function _M:SetMoveDisp(disp)
    self._move_disp = disp
end

---@param disp Vec2
function _M:AddMoveDisp(disp)
    self._move_disp = self._move_disp + disp
end

-- The game object owning a physics shape, or nil when the shape belongs to
-- static geometry (a wall, an obstacle...) without a game object script.
---@param shape PhysicsShape
---@return GameObject|nil
local function findShapeOwner(shape)
    local script = TL_Common.GetContext():GetScriptManager():Get(shape:GetOwner())
    if not script or not script.GetGameObject then
        return nil
    end
    return script:GetGameObject()
end

-- Static geometry doesn't bounce: only the velocity part pushing into the
-- surface is dropped, so the character keeps its speed along the wall and
-- slides instead of sticking to it.
---@private
---@param normal Vec2
function _M:slideAlongNormal(normal)
    local length_squared = normal:LengthSquared()
    if length_squared == 0 then
        return
    end

    local unit_normal = normal / math.sqrt(length_squared)
    self.m_velocity =
        self.m_velocity - unit_normal * self.m_velocity:Dot(unit_normal)
end

---@private
function _M:slideAlongTouchedSurfaces()
    if not self.m_cct then
        return
    end

    for i = 0, self.m_cct:GetTouchedShapeCount() - 1 do
        self:slideAlongNormal(self.m_cct:GetTouchedNormalAt(i))
    end
end

-- The character this component is touching, if any. Every touched shape is
-- checked and not only the last one: standing against a wall, the last hit is
-- often the wall itself, which used to hide the character in front of it.
---@private
---@return GameObject|nil
function _M:findTouchedCharacter()
    if not self.m_cct then
        return nil
    end

    for i = 0, self.m_cct:GetTouchedShapeCount() - 1 do
        local shape = self.m_cct:GetTouchedShapeAt(i)
        local owner = shape and findShapeOwner(shape)
        if owner then
            ---@cast owner ClientGameObject|ServerGameObject
            local move = owner.m_move_component
            if move and move ~= self then
                return owner
            end
        end
    end
    return nil
end

--- Whether this component owns the collision response. The server is the
--- authority: a client must not push itself, otherwise the push is applied
--- twice (once by the client in its reported displacement and once by the
--- server velocity) and the two sides drift apart.
---@return boolean
function _M:IsCollisionImpulseAuthority()
    return true
end

-- Push both characters apart when another character was touched: both
-- velocities are dropped and both get an opposite impulse along the line
-- between them. Touching static geometry only drops the velocity pushing into
-- it.
---@private
function _M:handleTouchedShape()
    local other = self:findTouchedCharacter()
    if not other then
        self:slideAlongTouchedSurfaces()
        return
    end
    ---@cast other ClientGameObject|ServerGameObject

    local other_move = other.m_move_component
    if not other_move then
        self:slideAlongTouchedSurfaces()
        return
    end

    if not self:IsCollisionImpulseAuthority() then
        return
    end

    -- Only the first frame of a contact pushes: re-zeroing and re-impulsing on
    -- every frame of a continuous contact makes both characters jitter.
    if self._collision_timer > 0 then
        return
    end
    self._collision_timer = self.m_collision_interval

    local self_position = self:GetGameObject().m_transform.m_position
    local from_self_to_other = other.m_transform.m_position - self_position
    local direction = self._last_move_dir
    if from_self_to_other:LengthSquared() > 0 then
        direction = from_self_to_other:Normalize()
    end

    self.m_velocity = TL_Common.Vec2.ZERO
    other_move:SetVelocity(TL_Common.Vec2.ZERO)
    self:AddImpulse(direction * -self.m_collision_impulse)
    other_move:AddImpulse(direction * self.m_collision_impulse)
end

-- The base consumes a per-frame displacement (`_move_disp`); the elapsed time
-- is accepted for interface consistency with the other component updates.
---@param elapse_time TimeType?
function _M:Update(elapse_time)
    if self._collision_timer > 0 then
        self._collision_timer = self._collision_timer - (elapse_time or 0)
    end

    local disp = self._move_disp
    if disp:LengthSquared() == 0 then
        return
    end

    self._last_move_dir = disp:Normalize()

    local transform = self:GetGameObject().m_transform
    if self.m_cct then
        self.m_cct:MoveAndSlide(disp)
        local new_position = self.m_cct:GetPosition()
        transform.m_position = new_position
    else
        transform.m_position = transform.m_position + disp
    end

    self:StopMove()
    self:handleTouchedShape()
end

return _M
