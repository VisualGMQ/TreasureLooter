local HpComponent = require("common.components.hp")

---@class ClientHpComponentDefinition
---@field m_hp number
---@field m_invincible_time TimeType
---@field m_anim_player AnimationPlayer
---@field m_hurt_anim AnimationHandle
---@field m_dead_anim AnimationHandle

---@class ClientHpComponent : HpComponent
---@field _anim_player AnimationPlayer
---@field _hurt_anim AnimationHandle
---@field _dead_anim AnimationHandle
---@field _defer_dead_timer Timer
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = HpComponent })

---@param gameobject any
---@param definition ClientHpComponentDefinition
---@return ClientHpComponent
function _M.new(gameobject, definition)
    local self = HpComponent.new(gameobject, definition.m_hp, definition.m_invincible_time)
    ---@cast self ClientHpComponent
    self._anim_player = definition.m_anim_player
    self._hurt_anim = definition.m_hurt_anim
    self._dead_anim = definition.m_dead_anim
    local ctx = TL_Client.GetContext()
    self._invincible_timer:SetTimerListener(function(event)
        self:onStopInvincible(event)
    end)
    local defer_dead_time = self._dead_anim:GetFinishTime()
    self._defer_dead_timer = ctx:GetTimerManager():Create(defer_dead_time, TL_Schema.TimerEventType.Cooldown, 0)
    self._defer_dead_timer:SetTimerListener(function(event)
        self:onDeferDead(event)
    end)

    return setmetatable(self, _M)
end

---@param damage number
function _M:Hurt(damage)
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

---@param event TimerEvent
function _M:onStopInvincible(event)
    self._anim_player:Stop()
end

---@param event TimerEvent
function _M:onDeferDead(event)
    self:GetGameObject():Destroy()
end

function _M:OnQuit()
    local ctx = TL_Client.GetContext()
    ctx:GetTimerManager():Remove(self._defer_dead_timer)
    HpComponent.OnQuit(self)
end

return _M
