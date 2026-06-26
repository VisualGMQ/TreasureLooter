local Component = require("@common/components/component")

local _M = {}
_M.__index = _M
setmetatable(_M, {__index = Component})


type HpComponentData = {
    m_hp: number,

    _invincible_timer:Timer,
}
export type HpComponent = typeof(_M) & HpComponentData & Component.Component

function _M.new(gameobject, hp:number, invincible_time:TimeType):HpComponent
    local self = Component.new(gameobject) :: HpComponent
    self.m_hp = hp

    local ctx = TL_Client.GetContext()
    self._invincible_timer = ctx:GetTimerManager():Create(invincible_time, TL_Schema.TimerEventType.Cooldown, 0)
    return setmetatable(self, _M)
end

function _M.Hurt(self:HpComponent, damage: number)
    if self:IsInvincible() then
        return
    end

    self.m_hp -= damage
    self._invincible_timer:Start()
end

function _M.IsDead(self:HpComponent):boolean
    return self.m_hp <= 0
end

function _M.IsAlive(self:HpComponent):boolean
    return not self:IsDead()
end

function _M.IsInvincible(self:HpComponent):boolean
    return self._invincible_timer:IsRunning()
end

function _M.OnQuit(self:HpComponent)
    local ctx = TL_Common.GetContext()
    ctx:GetTimerManager():Remove(self._invincible_timer)
end


return _M