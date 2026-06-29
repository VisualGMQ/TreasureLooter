local Component = require("@common/components/component")

local _M = {}
_M.__index = _M
setmetatable(_M, Component)

type ConstructableComponentData = {
	_materials: { [DID]: number },
	_phases: { any },
	_index: number,
	_material_accept_range: number,
	_material_trigger: Trigger,
	_material_trigger_event_id: EventListenerID,
	_fx_on_accept_material: DID,
}

export type ConstructableComponent = typeof(_M) & ConstructableComponentData & Component.Component

export type ConstructableComponentDefinition = {
	m_phases: { any },
	m_material_accept_range: number,
	m_fx_on_accept_material: DID,
}

function _M.new(gameobject, definition: ConstructableComponentDefinition): ConstructableComponent
	local self = Component.new(gameobject) :: ConstructableComponent
	self._materials = {}
	self._phases = definition.m_phases or {}
	self._index = 0
	self._material_accept_range = definition.m_material_accept_range or 50
	self._fx_on_accept_material = definition.m_fx_on_accept_material or TL_Schema.DID.Invalid

	local ctx = TL_Common.GetContext()
	local entity = gameobject:GetEntity()

	-- create material detector
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

	self._material_trigger_event_id = ctx:GetEventSystem():AddTriggerEnterEvent(
		function(id, event: TriggerEnterEvent)
			self:onMaterialEnter(id, event)
		end)

	return setmetatable(self, _M)
end

function _M.onMaterialEnter(self: ConstructableComponent, id: EventListenerID, event: TriggerEnterEvent)
	if event:GetSrcEntity() ~= self._material_trigger:GetOwner() then
		return
	end

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

	-- 检查当前阶段是否需要这个材料
	if self._index == 0 or self._index >= #self._phases then
		return
	end

	local phase = self._phases[self._index]
	if not phase then
		return
	end

	local needed = false
	for _, mat in ipairs(phase.m_materials) do
		if mat.m_material_did == material_did then
			needed = true
			break
		end
	end

	if not needed then
		return
	end

	self:AddMaterial(go)
	go:Destroy()

	if self._fx_on_accept_material ~= TL_Schema.DID.Invalid then
		local ClientWorld = require("@client/world")
		local pos = self._gameobject.m_transform:GetGlobalPosition()
		pos.y += 10
		ClientWorld.GetInst():AddFX(self._fx_on_accept_material, pos)
	end
end

function _M.AddMaterial(self: ConstructableComponent, material_gameobject: any)
	local did = material_gameobject:GetDID()
	self._materials[did] = (self._materials[did] or 0) + 1

	-- 检查当前阶段是否完成
	if self._index == 0 or self._index >= #self._phases then
		return
	end

	local phase = self._phases[self._index]
	if not phase then
		return
	end

	local complete = true
	for _, m in ipairs(phase.m_materials) do
		if (self._materials[m.m_material_did] or 0) < m.m_count then
			complete = false
			break
		end
	end

	if complete then
		self:StepToNextPhase()
	end
end

function _M.AcceptMaterial(self: ConstructableComponent, material_gameobject: any): boolean
	if self._index == 0 or self._index >= #self._phases then
		return false
	end

	local phase = self._phases[self._index]
	if not phase then
		return false
	end

	local item_component = material_gameobject.m_item_component
	if not item_component then
		return false
	end

	local material_did = material_gameobject:GetDID()
	local needed = false

	for _, mat in ipairs(phase.m_materials) do
		if mat.m_material_did == material_did then
			needed = true
			self._materials[material_did] = (self._materials[material_did] or 0) + 1

			material_gameobject:Destroy()

			local complete = true
			for _, m in ipairs(phase.m_materials) do
				if (self._materials[m.m_material_did] or 0) < m.m_count then
					complete = false
					break
				end
			end

			if complete then
				self:StepToNextPhase()
			end

			return true
		end
	end

	if not needed then
		return false
	end

	return false
end

function _M.StepToNextPhase(self: ConstructableComponent)
	self._index += 1
	self._materials = {}

	if self._index > #self._phases then
		return
	end

	local next_phase = self._phases[self._index]
	if next_phase then
		local client_ctx = TL_Client.GetContext()
		local entity = self._gameobject:GetEntity()
		local sprite = client_ctx:GetSpriteManager():Get(entity)
		if sprite then
			sprite.m_image = next_phase.m_sprite.m_image
			sprite.m_region = next_phase.m_sprite.m_region
			sprite.m_size = next_phase.m_sprite.m_size
			sprite.m_anchor = next_phase.m_sprite.m_anchor
			sprite.m_color = next_phase.m_sprite.m_color
			sprite.m_flip = next_phase.m_sprite.m_flip
		end
	end
end

function _M.GetCurrentPhaseIndex(self: ConstructableComponent): number
	return self._index
end

function _M.OnQuit(self: ConstructableComponent)
	local ctx = TL_Common.GetContext()
	ctx:GetEventSystem():Remove(self._material_trigger_event_id)
end

return _M
