local Component = require("@common/components/component")

local _M = {}
_M.__index = _M
setmetatable(_M, Component)

type FXComponentData = {
    _anim_player:AnimationPlayer,
}

export type FXComponent = typeof(_M) & FXComponentData & Component.Component

export type FXComponentDefinition = {
    m_anim_player:AnimationPlayer,
}

function _M.new(gameobject, definition:FXComponentDefinition):FXComponent
    local self = Component.new(gameobject) :: FXComponent
    self._anim_player = definition.m_anim_player
    return setmetatable(self, _M)
end

function _M.Play(self:FXComponent)
    self._anim_player:Play()
end

function _M.Stop(self:FXComponent)
    self._anim_player:Stop()
end

function _M.Rewind(self:FXComponent)
    self._anim_player:Rewind()
end

function _M.Pause(self:FXComponent)
    self._anim_player:Pause()
end

function _M.Enable(self:FXComponent)
    local ctx = TL_Client.GetContext()
    ctx:GetSpriteManager():Enable(self:GetGameObject():GetEntity())
end

function _M.Disable(self:FXComponent)
    local ctx = TL_Client.GetContext()
    ctx:GetSpriteManager():Disable(self:GetGameObject():GetEntity())
    self._anim_player:Stop()
end

function _M.SetColor(self:FXComponent, r:number, g:number, b:number, a:number)
    local ctx = TL_Client.GetContext()
    local sprite = ctx:GetSpriteManager():Get(self:GetGameObject():GetEntity())
    if sprite then
        sprite.m_color = TL_Common.Color(r, g, b, a)
    end
end

return _M
