local Component = require("@common/components/component")
local _M = {
    k_init_forward_dir = TL_Common.Vec2.Y_UNIT * -1
}
_M.__index = _M
setmetatable(_M, Component)

type AttackComponentData = {
    m_damage:number,
    m_attack_anim:AnimationHandle,
    m_hit_area:Trigger,
    m_cooldown:Timer,

    _anim_player:AnimationPlayer,
    _transform:Transform,
    _can_attack:boolean,
    _cooldown_timer_event_id:EventListenerID,
    _hit_area_event_id:EventListenerID,
    _aim_dir:Vec2,
}

export type AttackComponent = typeof(_M) & AttackComponentData & Component.Component

export type AttackComponentDefinition = {
    m_damage:number,
    m_attack_anim:AnimationHandle,
    m_hit_area:Trigger,
    m_cooldown:Timer,   
    m_anim_player:AnimationPlayer,
    m_transform:Transform,
}

function _M.new(gameobject, definition:AttackComponentDefinition):AttackComponent
    local self = Component.new(gameobject) :: AttackComponent
    self.m_attack_anim = definition.m_attack_anim
    self.m_damage = definition.m_damage
    self.m_hit_area = definition.m_hit_area
    self.m_cooldown = definition.m_cooldown
    self._anim_player = definition.m_anim_player
    self._transform = definition.m_transform
    self._can_attack = true
    self._aim_dir = self.k_init_forward_dir

    local ctx = TL_Common.GetContext()
    local event_system = ctx:GetEventSystem()
    self._hit_area_event_id = event_system:AddTriggerEnterEvent(
        function(id:EventListenerID, event:TriggerEnterEvent)
            self:onAttack(id, event)
        end
    )
    local timer_mgr = ctx:GetTimerManager()
    self.m_cooldown = timer_mgr:Create(self.m_cooldown, TL_Schema.TimerEventType.Cooldown, 0)
    self._cooldown_timer_event_id = event_system:AddTimerEvent(
        function(id:EventListenerID, event:TimerEvent)
            self:onAttackCooldown(id, event)
        end
    )
    return setmetatable(self, _M)
end

function _M.GetAttackDegrees(self:AttackComponent)
    return self._transform.m_rotation
end

function _M.SetAimDir(self:AttackComponent, dir: Vec2)
    self._aim_dir = dir
    self._transform.m_rotation = TL_Common.Degrees(TL_Common.GetAngle(self.k_init_forward_dir, self._aim_dir))
end

function _M.Attack(self:AttackComponent)
    if not self._can_attack then
        return
    end

    self._anim_player:ChangeAnimation(self.m_attack_anim)
    self._anim_player:SetLoop(0)
    self._anim_player:Play()
    self._transform.m_rotation = TL_Common.Degrees(TL_Common.GetAngle(self.k_init_forward_dir, self._aim_dir))
    self._can_attack = false

    self.m_cooldown:Start()
end

function _M.onAttack(self:AttackComponent, id:EventListenerID, event:TriggerEnterEvent)
    if event:GetSrcEntity() ~= self.m_hit_area:GetOwner() then
        return
    end
    local result = event:GetOverlapResult()
    local ctx = TL_Common.GetContext()
    local script = ctx:GetScriptManager():Get(result.m_dst_entity)

    if not script then
        return
    end

    if script.m_gameobject.m_hp_component then
        script.m_gameobject.m_hp_component:Hurt(self.m_damage)
    end
end

function _M.onAttackCooldown(self:AttackComponent, id:EventListenerID, event:TimerEvent)
    if event:GetID() ~= self.m_cooldown:GetID() then
        return
    end

    self._can_attack = true
end

function _M.OnQuit(self:AttackComponent)
    local ctx = TL_Common.GetContext()
    ctx:GetEventSystem():Remove(self._cooldown_timer_event_id)
    ctx:GetEventSystem():Remove(self._hit_area_event_id)
end

return _M