local HpComponent = require("common.components.hp")

---@class ClientHpComponentDefinition
---@field m_hp number
---@field m_invincible_time TimeType
---@field m_anim_player AnimationPlayer
---@field m_hurt_anim AnimationHandle
---@field m_dead_anim AnimationHandle

---@class ClientHpComponent : HpComponent
---@field _anim_player AnimationPlayer?
---@field _hurt_anim AnimationHandle
---@field _dead_anim AnimationHandle
---@field _defer_dead_timer Timer?
---@field private _dead_visual_started boolean
---@field private _quitted boolean
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
    self._dead_visual_started = false
    self._quitted = false
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

--- Local prediction path: applies `damage` honouring the invincibility window
--- and starts the invincibility timer (through `HpComponent.Hurt`).
---@param damage number
function _M:Hurt(damage)
    if self._quitted or self:IsInvincible() then
        return
    end

    local old_hp = self.m_hp
    HpComponent.Hurt(self, damage)
    if old_hp <= 0 then
        return
    end

    if self:IsDead() then
        self:applyDeadVisual()
    else
        self:applyHurtVisual()
    end
end

--- Server-authoritative hp sync (`Hurt` message). Unlike `Hurt` it ignores
--- the local invincibility window: the server decides the hp value. `hp > 0`
--- restarts a fresh invincibility blink, `hp <= 0` starts the death
--- presentation. Idempotent: once dying/dead it does nothing.
---@param hp number
function _M:SyncHurt(hp)
    if self._quitted or self._dead_visual_started then
        return
    end

    self.m_hp = hp
    if self:IsDead() then
        self:applyDeadVisual()
    else
        -- `Start` does not reset the elapsed time, so rewind first to get a
        -- full invincibility window on every authoritative hit.
        self._invincible_timer:Stop()
        self._invincible_timer:Start()
        self:applyHurtVisual()
    end
end

--- Force the death presentation whatever the current hp is (`Kill` message).
--- Idempotent.
function _M:ForceDead()
    if self._quitted or self._dead_visual_started then
        return
    end

    self.m_hp = 0
    self:applyDeadVisual()
end

---@return number
function _M:GetHp()
    return self.m_hp
end

--- Play the hurt animation as an infinite blink for the invincibility window.
---@private
function _M:applyHurtVisual()
    if not self._anim_player then
        return
    end

    self._anim_player:ChangeAnimation(self._hurt_anim)
    self._anim_player:SetLoop(-1)
    self._anim_player:Play()
end

--- Play the death animation once and defer the destroy to its end.
---@private
function _M:applyDeadVisual()
    if self._quitted or self._dead_visual_started then
        return
    end
    self._dead_visual_started = true

    -- Stop the invincibility timer so its stop listener cannot cut the dead
    -- animation short when the blink was already about to expire.
    self._invincible_timer:Stop()
    self:disableCollision()

    if self._anim_player then
        self._anim_player:ChangeAnimation(self._dead_anim)
        self._anim_player:SetLoop(0)
        self._anim_player:Play()
    end
    if self._defer_dead_timer then
        self._defer_dead_timer:Start()
    end
end

---@param event TimerEvent
function _M:onStopInvincible(event)
    if self._quitted or self._dead_visual_started or not self._anim_player then
        return
    end
    self._anim_player:Stop()
end

---@param event TimerEvent
function _M:onDeferDead(event)
    local go = self:GetGameObject()
    if self._quitted or not go then
        return
    end

    -- The entity can already be destroyed (the game scene was unloaded): only
    -- drop it when it still exists.
    if not TL_Client.GetContext():GetTransformManager():Has(go:GetEntity()) then
        return
    end

    go:Destroy()
end

function _M:OnQuit()
    self._quitted = true

    local ctx = TL_Client.GetContext()
    if self._defer_dead_timer then
        ctx:GetTimerManager():Remove(self._defer_dead_timer)
        self._defer_dead_timer = nil
    end

    HpComponent.OnQuit(self)
end

return _M
