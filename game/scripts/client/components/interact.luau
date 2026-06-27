local InteractComponent = require("@common/components/interact")

local _M = {
    k_hint_offset = -16,
}
_M.__index = _M
setmetatable(_M, InteractComponent)

type ClientInteractComponentData = {
    m_hint_fx:Entity,
    _showing:boolean,
}

export type ClientInteractComponent = typeof(_M) & ClientInteractComponentData & InteractComponent.InteractComponent

export type ClientInteractComponentDefinition = { } & InteractComponent.InteractComponentDefinition

function _M.new(gameobject, definition:ClientInteractComponentDefinition):ClientInteractComponent
    local self = InteractComponent.new(gameobject, definition) :: ClientInteractComponent
    self.m_hint_fx = TL_Common.null_entity
    self._showing = false
    return setmetatable(self, _M)
end

function _M.SetHintFX(self:ClientInteractComponent, entity:Entity)
    self.m_hint_fx = entity
end

function _M.getHintFXComponent(self:ClientInteractComponent):any
    if self.m_hint_fx == TL_Common.null_entity then
        return nil
    end
    local script = TL_Client.GetContext():GetScriptManager():Get(self.m_hint_fx)
    if script and script.m_gameobject then
        return script.m_gameobject.m_fx_component
    end
    return nil
end

function _M.Update(self:ClientInteractComponent, elapse_time:TimeType)
    InteractComponent.Update(self, elapse_time)

    local fx_component = self:getHintFXComponent()
    if not fx_component then
        return
    end

    local interact_obj = self._interact_object
    local target = interact_obj or self._detect_object

    if target then
        local fx_transform = TL_Client.GetContext():GetTransformManager():Get(self.m_hint_fx)
        if fx_transform then
            fx_transform.m_position = target.m_transform:GetGlobalPosition() + TL_Common.Vec2(0, _M.k_hint_offset)
        end

        -- red when actually interactable, white when only seen in view
        if interact_obj then
            fx_component:SetColor(1, 0, 0, 1)
        else
            fx_component:SetColor(1, 1, 1, 1)
        end

        if not self._showing then
            fx_component:Enable()
            fx_component:Play()
            self._showing = true
        end
    else
        if self._showing then
            fx_component:Disable()
            self._showing = false
        end
    end
end

return _M
