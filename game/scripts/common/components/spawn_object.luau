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

function _M._returnController(self: SpawnObjectComponent, controller_entity: Entity)
	table.insert(self._idle_controllers, controller_entity)
end

function _M.Spawn(self: SpawnObjectComponent, mine_position: Vec2)
	if self._count == 0 then
		return
	end
	if self._count > 0 then
		self._count -= 1
	end

	local ClientCreation = require("@client/creation")
	local ClientSpawnedComponent = require("@client/components/spawned")

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

	local angle = math.random() * math.pi * 2
	local radius = math.random() * k_spawn_radius
	local target_x = math.cos(angle) * radius

	local definition = world.m_object_definitions:GetItem(self._object_did)
	local prefab = ClientCreation.ConvertItemDefinitionToPrefab(definition)
	prefab.m_client_script = k_default_script

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

	local controller_entity = self:borrowController(ctx, scene, mine_entity)

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

	local spawn_anim_indices = {}
	local multi_anim_player = ctx:GetMultiAnimationPlayerManager():Get(entity)
	if multi_anim_player and #self._spawn_animations > 0 then
		local total = multi_anim_player:GetAnimationCount()
		local start_idx = total - #self._spawn_animations
		for j = 0, #self._spawn_animations - 1 do
			table.insert(spawn_anim_indices, start_idx + j)
		end
	end

	local mineral_gameobject = script.m_gameobject :: any
	local spawner = ClientSpawnedComponent.new(mineral_gameobject, self._gameobject, controller_entity, target_x, spawn_anim_indices)
	mineral_gameobject.m_spawned_component = spawner

	if multi_anim_player then
		for _, idx in ipairs(spawn_anim_indices) do
			local anim = multi_anim_player:GetAnimation(idx)
			if anim then
				anim:Play()
			end
		end
	end
end

function _M.GetCount(self: SpawnObjectComponent): number
	return self._count
end

function _M.SetCount(self: SpawnObjectComponent, count: number)
	self._count = count
end

return _M
