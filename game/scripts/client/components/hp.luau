local HpComponent = require("@common/components/hp")

local _M = {}
_M.__index = _M
setmetatable(_M, {__index = HpComponent})

type ClientHpComponentData = {
    _anim_player:AnimationPlayer,
    _hurt_anim:AnimationHandle,
    _dead_anim:AnimationHandle,

    _invincible_event_id:EventListenerID,
    _defer_dead_timer:Timer,
    _defer_dead_event_id:EventListenerID,
}

export type ClientHpComponent = typeof(_M) & ClientHpComponentData & HpComponent.HpComponent

export type ClientHpComponentDefinition = {
    m_hp:number,
    m_invincible_time:TimeType,
    m_anim_player:AnimationPlayer,
    m_hurt_anim:AnimationHandle,
    m_dead_anim:AnimationHandle,
}

function _M.new(gameobject, definition:ClientHpComponentDefinition):ClientHpComponent
    local self = HpComponent.new(gameobject, definition.m_hp, definition.m_invincible_time) :: ClientHpComponent
    self._anim_player = definition.m_anim_player
    self._hurt_anim = definition.m_hurt_anim
    self._dead_anim = definition.m_dead_anim
    local ctx = TL_Client.GetContext()
    self._invincible_event_id = ctx:GetEventSystem():AddTimerEvent(
        function(id, event)
            self:onStopInvincible(id, event)
        end)
    local defer_dead_time = self._dead_anim:GetFinishTime()
    self._defer_dead_timer = ctx:GetTimerManager():Create(defer_dead_time, TL_Schema.TimerEventType.Cooldown, 0)
    self._defer_dead_event_id =  ctx:GetEventSystem():AddTimerEvent(
        function(id, event)
            self:onDeferDead(id, event)
        end)

    return setmetatable(self, _M)
end

function _M.Hurt(self:ClientHpComponent, damage:number)
    if self:IsInvincible() then
        return
    end

    local old_hp = self.m_hp
    HpComponent.Hurt(self, damage)
    if old_hp > 0 then
        if self:IsDead() then
            self._anim_player:ChangeAnimation(self._dead_anim)
            self._defer_dead_timer:Start()
            self._anim_player:SetLoop(0)
        else
            self._anim_player:ChangeAnimation(self._hurt_anim)
            self._anim_player:SetLoop(-1)
        end
        self._anim_player:Play()
    end
end

function _M.onStopInvincible(self:ClientHpComponent, id:EventListenerID, event:TimerEvent)
    if event:GetID() ~= self._invincible_timer:GetID() then
        return
    end

    self._anim_player:Stop()
end

function _M.onDeferDead(self:ClientHpComponent, id:EventListenerID, event:TimerEvent)
    if event:GetID() ~= self._defer_dead_timer:GetID() then
        return
    end

    TL_Client.GetContext():Log("destroy")
    self:GetGameObject():Destroy()
end

function _M.OnQuit(self:ClientHpComponent)
    local ctx = TL_Client.GetContext()
    ctx:GetEventSystem():Remove(self._invincible_event_id)
    ctx:GetTimerManager():Remove(self._defer_dead_timer)
    ctx:GetEventSystem():Remove(self._defer_dead_event_id)
    HpComponent.OnQuit(self)
end

return _M

