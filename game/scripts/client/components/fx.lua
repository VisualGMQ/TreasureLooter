local Component = require("common.components.component")

---@class FXComponentDefinition
---@field m_anim_player AnimationPlayer

---@class FXComponent : Component
---@field _anim_player AnimationPlayer
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Component })

---@param gameobject any
---@param definition FXComponentDefinition
---@return FXComponent
function _M.new(gameobject, definition)
    local self = Component.new(gameobject)
    ---@cast self FXComponent
    self._anim_player = definition.m_anim_player
    return setmetatable(self, _M)
end

function _M:Play()
    self._anim_player:Play()
end

function _M:Stop()
    self._anim_player:Stop()
end

function _M:Rewind()
    self._anim_player:Rewind()
end

function _M:Pause()
    self._anim_player:Pause()
end

function _M:Enable()
    local ctx = TL_Client.GetContext()
    ctx:GetSpriteManager():Enable(self:GetGameObject():GetEntity())
end

function _M:Disable()
    local ctx = TL_Client.GetContext()
    ctx:GetSpriteManager():Disable(self:GetGameObject():GetEntity())
    self._anim_player:Stop()
end

---@param r number
---@param g number
---@param b number
---@param a number
function _M:SetColor(r, g, b, a)
    local ctx = TL_Client.GetContext()
    local sprite = ctx:GetSpriteManager():Get(self:GetGameObject():GetEntity())
    if sprite then
        sprite.m_color = TL_Common.Color(r, g, b, a)
    end
end

return _M
