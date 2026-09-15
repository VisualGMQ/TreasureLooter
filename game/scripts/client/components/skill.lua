local SkillComponent = require("common.components.skill")

---@class ClientSkillComponentDefinition
---@field m_skill_definition SkillDefinitionHandle
---@field m_anim_player AnimationPlayer|nil

---@class ClientSkillComponent : SkillComponent
---@field _anim_player AnimationPlayer|nil
---@field _sprite Sprite|nil
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = SkillComponent })

---@param gameobject any
---@param definition ClientSkillComponentDefinition
---@return ClientSkillComponent
function _M.new(gameobject, definition)
    local self = SkillComponent.new(gameobject, definition.m_skill_definition)
    ---@cast self ClientSkillComponent
    self._anim_player = definition.m_anim_player
    self._sprite = nil
    return setmetatable(self, _M)
end

---@param caster Entity
function _M:Cast(caster)
    if self._is_casting then return end

    if not self._sprite then
        self._sprite = TL_Client.GetContext():GetSpriteManager():Get(self:GetGameObject():GetEntity())
    end
    if self._sprite then
        TL_Client.GetContext():GetSpriteManager():Enable(self:GetGameObject():GetEntity())
    end

    SkillComponent.Cast(self, caster)
end

function _M:Stop()
    if self._anim_player then
        self._anim_player:Stop()
    end
    if self._sprite then
        TL_Client.GetContext():GetSpriteManager():Disable(self:GetGameObject():GetEntity())
    end
    SkillComponent.Stop(self)
end

---@param carry_over number|nil
function _M:StepToNextPhase(carry_over)
    SkillComponent.StepToNextPhase(self, carry_over)

    if not self._is_casting then
        self:Stop()
        return
    end

    local phase = self:GetCurPhase()
    if self._anim_player and phase and phase._animation then
        self._anim_player:ChangeAnimation(phase._animation)
        self._anim_player:SetLoop(phase._loop)
        -- The anim player advances each frame on its own; we only need to set the
        -- start offset once to compensate the overshoot carried from the previous
        -- phase (if any), keeping the visual timing in sync with skill time.
        if carry_over and carry_over > 0 then
            self._anim_player:SetCurTime(carry_over)
        end
        self._anim_player:Play()
    end
end

---@param elapse_time TimeType
function _M:Update(elapse_time)
    SkillComponent.Update(self, elapse_time)
end

return _M
