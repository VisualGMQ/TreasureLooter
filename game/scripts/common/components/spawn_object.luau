local Component = require("@common/components/component")
local World = require("@common/world")

local ClientCreation

local _M = {}
_M.__index = _M
setmetatable(_M, Component)

type SpawningObject = {
	controller_entity: Entity,
	item_entity: Entity,
	target_x: number,
	elapsed: number,
	duration: number,
}

type SpawnObjectComponentData = {
	_object_did: DID,
	_count: number,
	_spawn_animations: { AnimationHandle },
	_spawning_objects: { SpawningObject },
	_idle_controllers: { Entity },
}

export type SpawnObjectComponent = typeof(_M) & SpawnObjectComponentData & Component.Component

export type SpawnObjectComponentDefinition = {
	m_object_did: DID,
	m_count: number,
	m_spawn_animations: { AnimationHandle },
}

local k_default_script = TL_Common.Path("scripts/client/gameobject_behavior.luau")
local k_spawn_radius = 16

function _M.new(gameobject, definition: SpawnObjectComponentDefinition): SpawnObjectComponent
	local self = Component.new(gameobject) :: SpawnObjectComponent
	self._object_did = definition.m_object_did
	self._count = definition.m_count
	self._spawn_animations = definition.m_spawn_animations or {}
	self._spawning_objects = {}
	self._idle_controllers = {}
	return setmetatable(self, _M)
end

-- 获取或创建一个空闲控制器，并挂载到矿下，位置重置为 (0,0)
function _M._borrowController(self: SpawnObjectComponent, ctx, scene, mine_entity): Entity
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

function _M.Spawn(self: SpawnObjectComponent, mine_position: Vec2)
	if self._count == 0 then
		return
	end
	if self._count > 0 then
		self._count -= 1
	end

	if not ClientCreation then
		ClientCreation = require("@client/creation")
	end

	local ctx = TL_Client.GetContext()
	local scene = ctx:GetSceneManager():GetCurrentScene()
	if not scene then
		return
	end

	local world = World.GetInst()
	if not world.m_object_definitions then
		return
	end

	local mine_entity = self._gameobject:GetEntity()

	-- 在矿物旁边小圆内随机选目标 X
	local angle = math.random() * math.pi * 2
	local radius = math.random() * k_spawn_radius
	local target_x = math.cos(angle) * radius

	local definition = world.m_object_definitions:GetItem(self._object_did)
	local prefab = ClientCreation.ConvertItemDefinitionToPrefab(definition)
	prefab.m_client_script = k_default_script

	-- add spawn_animations to prefab's MultiAnimationPlayer
	if #self._spawn_animations > 0 then
		local multi_anim = prefab.m_animations
		if not multi_anim then
			multi_anim = TL_Schema.MultiAnimationPlayerDefinition()
		end
		local anims = multi_anim.m_animations
		for _, anim in ipairs(self._spawn_animations) do
			local anim_player_def = TL_Schema.AnimationPlayerDefinition()
			anim_player_def.m_animation = anim
			anim_player_def.m_auto_play = false
			anim_player_def.m_loop = 0
			anim_player_def.m_rate = 1.0
			table.insert(anims, anim_player_def)
		end
		multi_anim.m_animations = anims
		prefab.m_animations = multi_anim
	end

	local controller_entity = self:_borrowController(ctx, scene, mine_entity)

	-- 生成物：在控制器下 (0,0)，动画 TransformPosition 只管 Y 弹跳
	local item_transform = TL_Common.Transform()
	item_transform.m_position = TL_Common.Vec2(0, 0)

	local entity = scene:Instantiate(prefab, item_transform)
	ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

	local ctl_relationship = ctx:GetRelationshipManager():Get(controller_entity)
	if ctl_relationship then
		local item_relationship = ctx:GetRelationshipManager():Get(entity)
		if item_relationship then
			item_relationship:RemoveFromParent()
		end
		ctl_relationship:AddChild(entity)
	end

	local script = ctx:GetScriptManager():Get(entity)
	if script then
		local go_definition = ClientCreation.ConvertItemDefinitionToGODefinition(entity, self._object_did, definition)
		script:initGameObject(go_definition)
	end

	-- play bounce animation
	local multi_anim_player = ctx:GetMultiAnimationPlayerManager():Get(entity)
	if multi_anim_player then
		local bounce_index = multi_anim_player:GetAnimationCount() - #self._spawn_animations
		local bounce_anim = multi_anim_player:GetAnimation(bounce_index)
		if bounce_anim then
			bounce_anim:Play()
		end
	end

	table.insert(self._spawning_objects, {
		controller_entity = controller_entity,
		item_entity = entity,
		target_x = target_x,
		elapsed = 0,
		duration = 1.0,
	})
end

function _M.Update(self: SpawnObjectComponent, elapse_time: TimeType)
	local ctx = TL_Common.GetContext()
	local transform_mgr = ctx:GetTransformManager()
	local mine_entity = self._gameobject:GetEntity()

	local i = 1
	while i <= #self._spawning_objects do
		local obj = self._spawning_objects[i]
		obj.elapsed += elapse_time

		if obj.elapsed >= obj.duration then
			-- 停止动画，防止动画帧覆盖后续 reparent 设置的位置
			local client_ctx = TL_Client.GetContext()
			local multi_anim = client_ctx:GetMultiAnimationPlayerManager():Get(obj.item_entity)
			if multi_anim then
				multi_anim:StopAll()
			end

			-- 将生成物从控制器挪到矿物直接子节点，保留世界坐标
			local item_transform = transform_mgr:Get(obj.item_entity)
			local mine_relationship = ctx:GetRelationshipManager():Get(mine_entity)
			local item_relationship = ctx:GetRelationshipManager():Get(obj.item_entity)
			if item_transform and mine_relationship and item_relationship then
				local global_pos = item_transform:GetGlobalPosition()
				item_relationship:RemoveFromParent()
				mine_relationship:AddChild(obj.item_entity)
				local mine_transform = transform_mgr:Get(mine_entity)
				if mine_transform then
					item_transform.m_position = global_pos - mine_transform:GetGlobalPosition()
				end
			end

			-- 控制器归还到空闲池
			table.insert(self._idle_controllers, obj.controller_entity)
			table.remove(self._spawning_objects, i)
			continue
		end

		local transform = transform_mgr:Get(obj.controller_entity)
		if transform then
			local t = obj.elapsed / obj.duration
			local lerp_x = obj.target_x * t
			transform.m_position = TL_Common.Vec2(lerp_x, 0)
		end

		i += 1
	end
end

function _M.GetCount(self: SpawnObjectComponent): number
	return self._count
end

function _M.SetCount(self: SpawnObjectComponent, count: number)
	self._count = count
end

return _M
