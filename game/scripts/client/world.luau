local World = require("@common/world")

local _M = {}
_M.__index = _M
setmetatable(_M, World)

type FXEntry = {
	entity: Entity,
	gameobject: any,
	anim_end_event_id: EventListenerID,
}

type ClientWorldData = {
	m_player_hint: Entity,
	_idle_fx_pool: { FXEntry },
}

export type ClientWorld = typeof(_M) & ClientWorldData & World.World

function _M.new():ClientWorld
	local self = World.new()::ClientWorld
	self.m_player_hint = TL_Common.null_entity
	self._idle_fx_pool = {}
	return setmetatable(self, _M)
end

function _M.reuseFX(self: ClientWorld, ctx, did: DID, local_pos: Vec2, object_definitions)
	local entry = self._idle_fx_pool[#self._idle_fx_pool]
	table.remove(self._idle_fx_pool)

	ctx:GetSpriteManager():Enable(entry.entity)
	entry.gameobject.m_transform.m_position = local_pos

	if entry.anim_end_event_id and entry.anim_end_event_id ~= TL_Common.null_event_listener_id then
		ctx:GetEventSystem():Remove(entry.anim_end_event_id)
	end

	local fx_definition = object_definitions:GetFX(did)
	if fx_definition then
		local anim_player = ctx:GetMultiAnimationPlayerManager():Get(entry.entity):GetAnimation(0)
		if anim_player then
			anim_player:ChangeAnimation(fx_definition.m_animation.m_animation)
		end
	end

	local anim_player = ctx:GetMultiAnimationPlayerManager():Get(entry.entity):GetAnimation(0)
	if anim_player then
		entry.anim_end_event_id = ctx:GetEventSystem():AddAnimationEndEvent(
			function(id, event: AnimationEndEvent)
				if event:GetAnimationPlayerID() == anim_player:GetID() then
					entry.gameobject.m_fx_component:Disable()
					table.insert(self._idle_fx_pool, entry)
				end
			end)
	end
	entry.gameobject.m_fx_component:Play()
end

function _M.createFX(self: ClientWorld, ctx, did: DID, local_pos: Vec2, object_definitions)
	local scene = ctx:GetSceneManager():GetCurrentScene()

	local spawn_info = TL_Schema.ObjectSpawnDefinition()
	spawn_info.m_did = did

	local ClientCreation = require("@client/creation")
	local entity = ClientCreation.CreateFX(ClientCreation, scene, spawn_info, local_pos, object_definitions)

	local land_relationship = ctx:GetRelationshipManager():Get(self.m_land_entity)
	if land_relationship then
		local item_relationship = ctx:GetRelationshipManager():Get(entity)
		if item_relationship then item_relationship:RemoveFromParent() end
		land_relationship:AddChild(entity)
	end

	local gameobject = ctx:GetScriptManager():Get(entity).m_gameobject :: any
	local new_entry: FXEntry = { entity = entity, gameobject = gameobject, anim_end_event_id = TL_Common.null_event_listener_id }

	local anim_player = ctx:GetMultiAnimationPlayerManager():Get(entity):GetAnimation(0)
	if anim_player then
		new_entry.anim_end_event_id = ctx:GetEventSystem():AddAnimationEndEvent(
			function(id, event: AnimationEndEvent)
				if event:GetAnimationPlayerID() == anim_player:GetID() then
					gameobject.m_fx_component:Disable()
					table.insert(self._idle_fx_pool, new_entry)
				end
			end)
	end
	gameobject.m_fx_component:Play()
end

function _M.AddFX(self: ClientWorld, did: DID, position: Vec2)
	local ctx = TL_Client.GetContext()
	local scene = ctx:GetSceneManager():GetCurrentScene()
	if not scene then return end

	local object_definitions = self.m_object_definitions
	if not object_definitions then return end

	local land_transform = ctx:GetTransformManager():Get(self.m_land_entity)
	local local_pos = position
	if land_transform then
		local_pos = position - land_transform:GetGlobalPosition()
	end

	if #self._idle_fx_pool > 0 then
		self:reuseFX(ctx, did, local_pos, object_definitions)
	else
		self:createFX(ctx, did, local_pos, object_definitions)
	end
end

return _M
