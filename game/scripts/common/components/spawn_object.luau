local Component = require("@common/components/component")
local World = require("@common/world")

local _M = {}
_M.__index = _M
setmetatable(_M, Component)

type SpawnObjectComponentData = {
	_object_did: DID,
	_count: number,
	_spawn_animations: { AnimationHandle },
	_idle_controllers: { Entity },
}

export type SpawnObjectComponent = typeof(setmetatable({}::SpawnObjectComponentData, _M)) & Component.Component

export type SpawnObjectComponentDefinition = {
	m_object_did: DID,
	m_count: number,
	m_spawn_animations: { AnimationHandle },
}

function _M.new(gameobject, definition: SpawnObjectComponentDefinition): SpawnObjectComponent
	local self = Component.new(gameobject) :: SpawnObjectComponent
	self._object_did = definition.m_object_did
	self._count = definition.m_count
	self._spawn_animations = definition.m_spawn_animations or {}
	self._idle_controllers = {}
	return setmetatable(self, _M)
end

function _M.borrowController(self: SpawnObjectComponent, ctx, scene, mine_entity): Entity
	local controller_entity
	if #self._idle_controllers > 0 then
		controller_entity = self._idle_controllers[#self._idle_controllers]
		table.remove(self._idle_controllers)
		local ctl_transform = ctx:GetTransformManager():Get(controller_entity)
		if ctl_transform then
			ctl_transform.m_position = TL_Common.Vec2(0, 0)
		end
		return controller_entity
	end

	local prefab_mgr = ctx:GetAssetsManager():GetPrefabManager()
	local controller_prefab = prefab_mgr:Create()
	controller_prefab.m_transform = TL_Common.Transform()
	controller_prefab.m_draw_order = TL_Schema.DrawOrderDefinition()
	controller_prefab.m_draw_order.m_enable_y_sorting = true
	controller_prefab.m_draw_order.m_draw_layer = TL_Schema.DrawLayer.TilemapArch

	local controller_transform = TL_Common.Transform()
	controller_transform.m_position = TL_Common.Vec2(0, 0)

	controller_entity = scene:Instantiate(controller_prefab, controller_transform)
	prefab_mgr:Unload(controller_prefab)

	local mine_relationship = ctx:GetRelationshipManager():Get(mine_entity)
	if mine_relationship then
		local ctl_relationship = ctx:GetRelationshipManager():Get(controller_entity)
		if ctl_relationship then
			ctl_relationship:RemoveFromParent()
		end
		mine_relationship:AddChild(controller_entity)
	end

	return controller_entity
end

function _M.returnController(self: SpawnObjectComponent, controller_entity: Entity)
	table.insert(self._idle_controllers, controller_entity)
end

function _M.Spawn(self: SpawnObjectComponent, mine_position: Vec2, count: number?)
	local to_spawn = count or self._count
	if to_spawn < 0 then
		to_spawn = 1
	end
	for i = 1, to_spawn do
		self:_spawnOne(mine_position)
	end
end

function _M._spawnOne(self: SpawnObjectComponent, mine_position: Vec2)
	TL_Common.GetContext():Log("spawn object not supported on this side")
end

function _M.GetCount(self: SpawnObjectComponent): number
	return self._count
end

function _M.SetCount(self: SpawnObjectComponent, count: number)
	self._count = count
end

return _M
