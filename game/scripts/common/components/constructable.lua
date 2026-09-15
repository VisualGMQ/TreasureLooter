local Component = require("common.components.component")

---@class ConstructableComponent : Component
---@field _growth_amount number
---@field _phases table<DID, ConstructPhase>
---@field _index number
---@field _material_accept_range number
---@field _material_trigger Trigger
---@field _fx_on_accept_material DID
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Component })

---@param gameobject any
---@param definition ConstructableComponentDefinition
---@return ConstructableComponent
function _M.new(gameobject, definition)
    local self = Component.new(gameobject)
    ---@cast self ConstructableComponent
    self._growth_amount = 0
    self._phases = definition.m_phases or {}
    self._index = 0
    self._material_accept_range = definition.m_material_accept_range or 50
    self._fx_on_accept_material = definition.m_fx_on_accept_material or TL_Schema.DID.Invalid

    local ctx = TL_Common.GetContext()
    local entity = gameobject:GetEntity()

    local shape = ctx:GetAssetsManager():GetPhysicsShapeDefinitionManager():Create()
    shape.m_is_rect = false
    shape.m_circle.m_center = TL_Common.Vec2(0, 0)
    shape.m_circle.m_radius = self._material_accept_range

    local collision_mask = TL_Common.CollisionGroup()
    collision_mask:Add(TL_Schema.CollisionGroupType.Material)

    shape.m_collision_mask = collision_mask

    local trigger_def = TL_Schema.TriggerDefinition()
    trigger_def.m_event_type = TL_Schema.TriggerEventType.InteractableDetect
    local shapes = trigger_def.m_physics_shapes
    table.insert(shapes, shape)
    trigger_def.m_physics_shapes = shapes
    trigger_def.m_trig_every_frame_when_touch = false

    ctx:GetTriggerComponentManager():RegisterEntity(entity, trigger_def)
    self._material_trigger = ctx:GetTriggerComponentManager():Get(entity)

    self._material_trigger:SetEnterListener(function(event)
        self:onMaterialEnter(event)
    end)

    return setmetatable(self, _M)
end

---@param event TriggerEnterEvent
function _M:onMaterialEnter(event)
    local result = event:GetOverlapResult()
    local script = TL_Common.GetContext():GetScriptManager():Get(result.m_dst_entity)
    if not script or not script.m_gameobject then
        return
    end

    local go = script.m_gameobject
    local material_did = go:GetDID()
    if not material_did or material_did == TL_Schema.DID.Invalid then
        return
    end

    if self._index == 0 or self._index >= #self._phases then
        return
    end

    local phase = self._phases[self._index]
    if not phase then
        return
    end

    local mat_config = phase.m_materials[material_did]

    if not mat_config then
        return
    end

    self._growth_amount = self._growth_amount + mat_config.m_growth
    go:Destroy()

    self:tryStepToNextPhase()
end

---@param growth number
function _M:AddGrowth(growth)
    self._growth_amount = self._growth_amount + growth
    self:tryStepToNextPhase()
end

function _M:tryStepToNextPhase()
    if self._index == 0 or self._index >= #self._phases then
        return
    end

    local phase = self._phases[self._index]
    if not phase then
        return
    end

    if self._growth_amount >= phase.m_next_phase_growth then
        self:StepToNextPhase()
    end
end

function _M:StepToNextPhase()
    self._index = self._index + 1
    self._growth_amount = 0

    if self._index > #self._phases then
        return
    end
end

---@return integer
function _M:GetCurrentPhaseIndex()
    return self._index
end

---@return boolean
function _M:IsAtLastPhase()
    return self._index >= #self._phases
end

---@param target_index integer
function _M:ResetToPhase(target_index)
    if target_index < 1 or target_index > #self._phases then
        return
    end

    self._index = target_index
    self._growth_amount = 0
end

return _M
