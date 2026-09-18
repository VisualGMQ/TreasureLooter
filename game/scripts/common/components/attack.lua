local Component = require("common.components.component")

---@class AttackComponentDefinition
---@field m_damage number
---@field m_attack_anim AnimationHandle
---@field m_hit_area Trigger
---@field m_cooldown TimeType
---@field m_anim_player AnimationPlayer
---@field m_transform Transform

---@class AttackComponent : Component
---@field m_damage number
---@field m_attack_anim AnimationHandle
---@field m_hit_area Trigger
---@field m_cooldown Timer
---@field _anim_player AnimationPlayer
---@field _transform Transform
---@field _can_attack boolean
---@field _anim_end_event_id EventListenerID
---@field _aim_dir Vec2
local _M = {
    k_init_forward_dir = TL_Common.Vec2.Y_UNIT * -1
}
_M.__index = _M
setmetatable(_M, { __index = Component })

---@param gameobject any
---@param definition AttackComponentDefinition
---@return AttackComponent
function _M.new(gameobject, definition)
    local self = Component.new(gameobject)
    ---@cast self AttackComponent
    self.m_attack_anim = definition.m_attack_anim
    self.m_damage = definition.m_damage
    self.m_hit_area = definition.m_hit_area
    self._anim_player = definition.m_anim_player
    self._transform = definition.m_transform
    self._can_attack = true
    self._aim_dir = _M.k_init_forward_dir
    self.m_hit_area:Disable()

    local ctx = TL_Common.GetContext()
    local event_system = ctx:GetEventSystem()
    self.m_hit_area:SetEnterListener(function(event)
        self:onAttack(event)
    end)
    local timer_mgr = ctx:GetTimerManager()
    self.m_cooldown = timer_mgr:Create(definition.m_cooldown, TL_Schema.TimerEventType.Cooldown, 0)
    self.m_cooldown:SetTimerListener(function(event)
        self:onAttackCooldown(event)
    end)
    self._anim_end_event_id = event_system:AddAnimationEndEvent(
        function(id, event)
            if event:GetAnimationPlayerID() == self._anim_player:GetID() then
                self.m_hit_area:Disable()
            end
        end
    )
    return setmetatable(self, _M)
end

---@return Degrees
function _M:GetAttackDegrees()
    return self._transform.m_rotation
end

---@param dir Vec2
function _M:SetAimDir(dir)
    self._aim_dir = dir
    self._transform.m_rotation = TL_Common.Degrees(TL_Common.GetAngle(self.k_init_forward_dir, self._aim_dir))
end

function _M:Attack()
    if not self._can_attack then
        return
    end

    self.m_hit_area:Enable()
    self._anim_player:ChangeAnimation(self.m_attack_anim)
    self._anim_player:SetLoop(0)
    self._anim_player:Play()
    self._transform.m_rotation = TL_Common.Degrees(TL_Common.GetAngle(self.k_init_forward_dir, self._aim_dir))
    self._can_attack = false

    self.m_cooldown:Start()
end

---@param event TriggerEnterEvent
function _M:onAttack(event)
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

---@param event TimerEvent
function _M:onAttackCooldown(event)
    self._can_attack = true
end

function _M:OnQuit()
    local ctx = TL_Common.GetContext()
    ctx:GetEventSystem():Remove(self._anim_end_event_id)
end

return _M
