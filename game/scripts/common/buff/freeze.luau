-- Freeze buff: locks the target so it can't act (move / attack) for a duration.
-- Enforcement is done by character behaviors, which skip their logic while the
-- BuffReceiveComponent reports IsFrozen(); this module tracks the timer and
-- keeps the target stopped defensively.
local _M = {}
_M.__index = _M

type FreezeData = {
    _gameobject: any,
    _type: BuffType,
    _remaining: number,
}

export type Freeze = typeof(_M) & FreezeData

function _M.new(buff_definition: BuffDefinition, gameobject: any): Freeze
    local self = {} :: FreezeData
    self._gameobject = gameobject
    self._type = buff_definition.m_type
    self._remaining = buff_definition.m_duration
    return setmetatable(self, _M)
end

function _M.GetType(self: Freeze): BuffType
    return self._type
end

function _M.Reset(self: Freeze, buff_definition: BuffDefinition)
    self._remaining = buff_definition.m_duration
end

function _M.Update(self: Freeze, elapse_time: TimeType)
    local move = self._gameobject.m_move_component
    if move then
        move:StopMove()
    end
    self._remaining -= elapse_time
end

function _M.IsFinished(self: Freeze): boolean
    return self._remaining <= 0
end

function _M.OnRemove(self: Freeze)
end

return _M
