-- Freeze buff: locks the target so it can't act (move / attack) for a duration.
-- Enforcement is done by character behaviors, which skip their logic while the
-- BuffReceiveComponent reports IsFrozen(); this module tracks the timer and
-- keeps the target stopped defensively.

---@class Freeze
---@field _gameobject any
---@field _type integer
---@field _remaining number
local _M = {}
_M.__index = _M

---@param buff_definition BuffDefinition
---@param gameobject any
---@return Freeze
function _M.new(buff_definition, gameobject)
    local self = setmetatable({}, _M)
    self._gameobject = gameobject
    self._type = buff_definition.m_type
    self._remaining = buff_definition.m_duration
    return self
end

---@return integer
function _M:GetType()
    return self._type
end

---@param buff_definition BuffDefinition
function _M:Reset(buff_definition)
    self._remaining = buff_definition.m_duration
end

---@param elapse_time TimeType
function _M:Update(elapse_time)
    local move = self._gameobject.m_move_component
    if move then
        move:StopMove()
    end
    self._remaining = self._remaining - elapse_time
end

---@return boolean
function _M:IsFinished()
    return self._remaining <= 0
end

function _M:OnRemove()
end

return _M
