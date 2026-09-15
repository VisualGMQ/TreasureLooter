local Component = require("common.components.component")

---@class HpComponent : Component
---@field m_hp number
---@field _invincible_timer Timer
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Component })

---@param gameobject any
---@param hp number
---@param invincible_time TimeType
---@return HpComponent
function _M.new(gameobject, hp, invincible_time)
    local self = Component.new(gameobject)
    ---@cast self HpComponent
    self.m_hp = hp

    local ctx = TL_Common.GetContext()
    self._invincible_timer = ctx:GetTimerManager():Create(invincible_time, TL_Schema.TimerEventType.Cooldown, 0)
    return setmetatable(self, _M)
end

---@param damage number
function _M:Hurt(damage)
    if self:IsInvincible() then
        return
    end

    self.m_hp = self.m_hp - damage
    self._invincible_timer:Start()

    if self:IsDead() then
        self:disableCollision()
    end
end

-- when dead the body should not block other characters anymore
function _M:disableCollision()
    local ctx = TL_Common.GetContext()
    local entity = self:GetGameObject():GetEntity()

    local cct_mgr = ctx:GetCCTManager()
    if cct_mgr:Has(entity) then
        cct_mgr:Disable(entity)
    end

    local static_collision_mgr = ctx:GetStaticCollisionManager()
    if static_collision_mgr:Has(entity) then
        static_collision_mgr:Disable(entity)
    end
end

---@return boolean
function _M:IsDead()
    return self.m_hp <= 0
end

---@return boolean
function _M:IsAlive()
    return not self:IsDead()
end

---@return boolean
function _M:IsInvincible()
    return self._invincible_timer:IsRunning()
end

function _M:OnQuit()
    local ctx = TL_Common.GetContext()
    ctx:GetTimerManager():Remove(self._invincible_timer)
end

return _M
