local HpComponent = require("common.components.hp")

--- The server has no animation player, so the definition only carries the
--- gameplay values of the character definition's HpComponentDefinition.
---@class ServerHpComponentDefinition
---@field m_hp number|nil
---@field m_invincible_time TimeType|nil

---@class ServerHpComponent : HpComponent
--- m_dead is set once the component died, so observers (e.g. the world) can
--- tell a real death from a hp that never changed.
---@field m_dead boolean
---@field private _dead_listener (fun(component: ServerHpComponent)|nil)
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = HpComponent })

local k_default_hp = 3
local k_default_invincible_time = 2

---@param gameobject any
---@param definition ServerHpComponentDefinition|nil
---@return ServerHpComponent
function _M.new(gameobject, definition)
    local hp = k_default_hp
    local invincible_time = k_default_invincible_time
    if definition then
        hp = definition.m_hp or k_default_hp
        invincible_time = definition.m_invincible_time or k_default_invincible_time
    end

    local self = HpComponent.new(gameobject, hp, invincible_time)
    ---@cast self ServerHpComponent
    self.m_dead = false
    self._dead_listener = nil
    return setmetatable(self, _M)
end

-- The damage is silently ignored while invincible (HpComponent.Hurt), so the
-- listener is only notified on the transition from alive to dead.
---@param damage number
function _M:Hurt(damage)
    local was_dead = self:IsDead()
    HpComponent.Hurt(self, damage)
    if (not was_dead) and self:IsDead() then
        self.m_dead = true
        if self._dead_listener then
            self._dead_listener(self)
        end
    end
end

---@return number
function _M:GetHp()
    return self.m_hp
end

---@return boolean
function _M:IsDead()
    if self.m_dead then
        return true
    end
    return HpComponent.IsDead(self)
end

---@param listener fun(component: ServerHpComponent)
function _M:SetDeadListener(listener)
    self._dead_listener = listener
end

return _M
