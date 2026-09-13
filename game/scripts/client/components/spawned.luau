local SpawnedComponent = require("@common/components/spawned")

local _M = {}
_M.__index = _M
setmetatable(_M, SpawnedComponent)

type ClientSpawnedComponentData = {
	_spawn_anim_indices: { number },
}

export type ClientSpawnedComponent = typeof(_M) & ClientSpawnedComponentData & SpawnedComponent.SpawnedComponent

function _M.new(gameobject, mine_gameobject, controller_entity: Entity, target_x: number, spawn_anim_indices: { number }): ClientSpawnedComponent
	local self = SpawnedComponent.new(gameobject, mine_gameobject, controller_entity, target_x) :: ClientSpawnedComponent
	self._spawn_anim_indices = spawn_anim_indices
	return setmetatable(self, _M)
end

function _M.Stop(self: ClientSpawnedComponent)
	local client_ctx = TL_Client.GetContext()
	local entity = self._gameobject:GetEntity()
	local anim_mgr = client_ctx:GetAnimationPlayerManager()
	if anim_mgr:Has(entity) then
		for _, idx in ipairs(self._spawn_anim_indices) do
			local anim = anim_mgr:Get(entity, idx)
			if anim then
				anim:Stop()
			end
		end
	end
	SpawnedComponent.Stop(self)
end

return _M
