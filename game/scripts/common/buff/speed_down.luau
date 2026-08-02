-- SpeedDown buff: slows down the target's movement while active.
local _M = {}
_M.__index = _M

-- Default multiplier applied to the target's movement speed while active,
-- used when the BuffDefinition doesn't specify m_speed_slow_factor.
local k_default_speed_factor = 0.5

type SpeedDownData = {
    _gameobject: any,
    _type: BuffType,
    _remaining: number,
    _factor: number,
    _origin_speed: number?,
}

export type SpeedDown = typeof(_M) & SpeedDownData

function _M.new(buff_definition: BuffDefinition, gameobject: any): SpeedDown
    local self = {} :: SpeedDownData
    self._gameobject = gameobject
    self._type = buff_definition.m_type
    self._remaining = buff_definition.m_duration
    self._factor = buff_definition.m_speed_slow_factor or k_default_speed_factor

    self._origin_speed = nil
    local move = gameobject.m_move_component
    if move then
        self._origin_speed = move:GetSpeed()
    end

    return setmetatable(self, _M)
end

function _M.GetType(self: SpeedDown): BuffType
    return self._type
end

-- Refresh this buff from a (possibly new) definition, keeping the captured
-- original speed so repeated applications don't compound the slowdown.
function _M.Reset(self: SpeedDown, buff_definition: BuffDefinition)
    self._remaining = buff_definition.m_duration
    self._factor = buff_definition.m_speed_slow_factor or k_default_speed_factor
end

function _M.Update(self: SpeedDown, elapse_time: TimeType)
    local move = self._gameobject.m_move_component
    if move and self._origin_speed then
        move:ChangeSpeed(self._origin_speed * self._factor)
    end
    self._remaining -= elapse_time
end

function _M.IsFinished(self: SpeedDown): boolean
    return self._remaining <= 0
end

function _M.OnRemove(self: SpeedDown)
    local move = self._gameobject.m_move_component
    if move and self._origin_speed then
        move:ChangeSpeed(self._origin_speed)
    end
end

return _M
