-- SpeedDown buff: slows down the target's movement while active.

---@class SpeedDown
---@field _gameobject any
---@field _type integer
---@field _remaining number
---@field _factor number
---@field _origin_speed number|nil
local _M = {}
_M.__index = _M

-- Default multiplier applied to the target's movement speed while active,
-- used when the BuffDefinition doesn't specify m_speed_slow_factor.
local k_default_speed_factor = 0.5

---@param buff_definition BuffDefinition
---@param gameobject any
---@return SpeedDown
function _M.new(buff_definition, gameobject)
    local self = setmetatable({}, _M)
    self._gameobject = gameobject
    self._type = buff_definition.m_type
    self._remaining = buff_definition.m_duration
    self._factor = buff_definition.m_speed_slow_factor or k_default_speed_factor

    self._origin_speed = nil
    local move = gameobject.m_move_component
    if move then
        self._origin_speed = move:GetSpeed()
    end

    return self
end

---@return integer
function _M:GetType()
    return self._type
end

-- Refresh this buff from a (possibly new) definition, keeping the captured
-- original speed so repeated applications don't compound the slowdown.
---@param buff_definition BuffDefinition
function _M:Reset(buff_definition)
    self._remaining = buff_definition.m_duration
    self._factor = buff_definition.m_speed_slow_factor or k_default_speed_factor
end

---@param elapse_time TimeType
function _M:Update(elapse_time)
    local move = self._gameobject.m_move_component
    if move and self._origin_speed then
        move:ChangeSpeed(self._origin_speed * self._factor)
    end
    self._remaining = self._remaining - elapse_time
end

---@return boolean
function _M:IsFinished()
    return self._remaining <= 0
end

function _M:OnRemove()
    local move = self._gameobject.m_move_component
    if move and self._origin_speed then
        move:ChangeSpeed(self._origin_speed)
    end
end

return _M
