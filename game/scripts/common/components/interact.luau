local Component = require("@common/components/component")
local GameObject = require("@common/gameobject")

local _M = {}
_M.__index = _M
setmetatable(_M, Component)

type InteractComponentData = {
    m_interact_area:Trigger,
    m_detect_range:number,
    m_can_interact_range:number,
    _interact_object:GameObject.GameObject?,
    _detect_object:GameObject.GameObject?,
}

export type InteractComponent = typeof(_M) & InteractComponentData & Component.Component

export type InteractComponentDefinition = {
    m_interact_area:Trigger,
    m_detect_range:number,
    m_can_interact_range:number,
}

function _M.new(gameobject, definition:InteractComponentDefinition):InteractComponent
    local self = Component.new(gameobject) :: InteractComponent
    self.m_interact_area = definition.m_interact_area
    self.m_detect_range = definition.m_detect_range
    self.m_can_interact_range = definition.m_can_interact_range
    self._interact_object = nil
    self._detect_object = nil
    return setmetatable(self, _M)
end

-- scan the detect trigger and split touched objects into:
--   _interact_object: nearest one within can_interact_range (truly interactable)
--   _detect_object:   nearest one beyond can_interact_range but inside detect_range (in view only)
function _M.Update(self:InteractComponent, elapse_time:TimeType)
    local ctx = TL_Common.GetContext()
    local script_mgr = ctx:GetScriptManager()
    local owner_pos = self._gameobject.m_transform:GetGlobalPosition()
    local can_interact_sq = self.m_can_interact_range * self.m_can_interact_range

    local nearest_interact = nil
    local nearest_interact_dist = math.huge
    local nearest_detect = nil
    local nearest_detect_dist = math.huge

    local shapes = self.m_interact_area:GetTouchingShapes()
    for _, shape in ipairs(shapes) do
        local ok, entity = pcall(function() return shape:GetOwner() end)
        if not ok or entity == TL_Common.null_entity then
            continue
        end
        local script = script_mgr:Get(entity)
        if script and script.m_gameobject then
            local go = script.m_gameobject
            local diff = go.m_transform:GetGlobalPosition() - owner_pos
            local dist = diff:LengthSquared()
            if dist <= can_interact_sq then
                if dist < nearest_interact_dist then
                    nearest_interact_dist = dist
                    nearest_interact = go
                end
            else
                if dist < nearest_detect_dist then
                    nearest_detect_dist = dist
                    nearest_detect = go
                end
            end
        end
    end

    self._interact_object = nearest_interact
    self._detect_object = nearest_detect
end

-- object within can_interact_range (truly interactable), or nil
function _M.GetInteractObject(self:InteractComponent):GameObject.GameObject?
    return self._interact_object
end

-- object inside detect_range but outside can_interact_range (seen in view), or nil
function _M.GetDetectObject(self:InteractComponent):GameObject.GameObject?
    return self._detect_object
end

return _M
