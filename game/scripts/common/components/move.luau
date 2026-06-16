local Component = require("@common/components/component")

local _M = {}
_M.__index = _M
setmetatable(_M, {__index = Component})

type MoveComponentData = {
    m_cct:CharacterController?,

    -- private fields
    _move_velocity:Vec2,
    _speed:number,
    _move_disp:number, -- this frame moved displacement
    _last_move_dir:Vec2,
}

export type MoveComponent = typeof(_M) & MoveComponentData & Component.Component

function _M.new(gameobject, cct:CharacterController?, speed:number):MoveComponent
    local self = Component.new(gameobject) :: MoveComponentData
    self._move_disp = 0
    self._move_velocity = TL_Common.Vec2.ZERO
    self.m_cct = cct
    self._speed = speed
    self._last_move_dir = TL_Common.Vec2.ZERO
    return setmetatable(self, _M)
end

function _M.GetLastMoveDirection(self:MoveComponent): Vec2
    return self._last_move_dir
end

function _M.ChangeSpeed(self:MoveComponent, speed: number)
    self._speed = speed
end

function _M.GetSpeed(self:MoveComponent):number
    return self._speed
end

function _M.SetDir(self:MoveComponent, dir:Vec2)
    self._move_velocity = dir * self._speed
end

function _M.GetVelocity(self:MoveComponent):Vec2
    return self._move_velocity
end

function _M.Teleport(self:MoveComponent, position:Vec2)
    if self.m_cct then
        self.m_cct:Teleport(position)
        local new_position = self.m_cct:GetPosition()
        self:GetGameObject().m_transform.m_position = new_position
    else
        self:GetGameObject().m_transform.m_position = position
    end
end

function _M.StopMove(self:MoveComponent)
    self._move_velocity = TL_Common.Vec2.ZERO
end

function _M.GetMovedDisp(self:MoveComponent):number
    return self._move_disp
end

function _M.IsWantMoving(self:MoveComponent):boolean
    return self._move_velocity:LengthSquared() > 0
end

function _M.Update(self:MoveComponent, elapse_time:TimeType)
    local disp = self._move_velocity * elapse_time
    self._move_disp = disp
    if disp:LengthSquared() == 0 then
        return
    end

    self._last_move_dir = self._move_velocity:Normalize()
    local transform = self:GetGameObject().m_transform
    if self.m_cct then
        self.m_cct:MoveAndSlide(disp)
        local new_position = self.m_cct:GetPosition()
        if new_position == transform.m_position then
            self._move_disp = 0
        end
        transform.m_position = new_position
    else
        transform.m_position += disp
    end
end


return _M