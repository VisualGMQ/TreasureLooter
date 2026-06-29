local Component = require("@common/components/component")

local _M = {}
_M.__index = _M
setmetatable(_M, Component)

type SpawnedComponentData = {
	_mine_gameobject: any,
	_controller_entity: Entity,
	_target_x: number,

	-- smooth move animation related
	_elapsed: number,
	_duration: number,
}

export type SpawnedComponent = typeof(_M) & SpawnedComponentData & Component.Component

function _M.new(gameobject, mine_gameobject, controller_entity: Entity, target_x: number): SpawnedComponent
	local self = Component.new(gameobject) :: SpawnedComponent
	self._mine_gameobject = mine_gameobject
	self._controller_entity = controller_entity
	self._target_x = target_x
	self._elapsed = 0
	self._duration = 1.0
	return setmetatable(self, _M)
end

function _M.Update(self: SpawnedComponent, elapse_time: TimeType)
	self._elapsed += elapse_time
	if self._elapsed >= self._duration then
		self:Stop()
		return
	end

	local ctx = TL_Common.GetContext()
	local transform = ctx:GetTransformManager():Get(self._controller_entity)
	if transform then
		local t = self._elapsed / self._duration
		transform.m_position = TL_Common.Vec2(self._target_x * t, 0)
	end
end

function _M.Stop(self: SpawnedComponent)
	local ctx = TL_Common.GetContext()
	local gameobject = self._gameobject
	local mine_spawn = self._mine_gameobject.m_spawn_object_component

	local item_entity = gameobject:GetEntity()
	local mine_entity = self._mine_gameobject:GetEntity()
	local transform_mgr = ctx:GetTransformManager()

	local item_transform = transform_mgr:Get(item_entity)
	local mine_relationship = ctx:GetRelationshipManager():Get(mine_entity)
	local item_relationship = ctx:GetRelationshipManager():Get(item_entity)
	if item_transform and mine_relationship and item_relationship then
		local global_pos = item_transform:GetGlobalPosition()
		item_relationship:RemoveFromParent()
		mine_relationship:AddChild(item_entity)
		local mine_transform = transform_mgr:Get(mine_entity)
		if mine_transform then
			item_transform.m_position = global_pos - mine_transform:GetGlobalPosition()
		end
	end

	mine_spawn:_returnController(self._controller_entity)

	gameobject.m_spawned_component = nil
end

return _M
