local Component = require("common.components.component")

---@class MoveComponent : Component
---@field m_cct CharacterController|nil
---@field private _move_disp Vec2
---@field private _last_move_dir Vec2
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
    self._move_disp = TL_Common.Vec2.ZERO
    self._last_move_dir = TL_Common.Vec2.ZERO
    self.m_cct = cct
    return setmetatable(self, _M)
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

-- The base consumes a per-frame displacement (`_move_disp`); the elapsed time
-- is accepted for interface consistency with the other component updates.
---@param elapse_time TimeType?
function _M:Update(elapse_time)
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
end

return _M
