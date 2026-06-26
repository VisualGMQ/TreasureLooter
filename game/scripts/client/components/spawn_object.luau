local SpawnObjectComponent = require("@common/components/spawn_object")

local _M = {}
_M.__index = _M
setmetatable(_M, SpawnObjectComponent)

type ClientSpawnObjectComponentData = { }

export type ClientSpawnObjectComponent = typeof(_M) & ClientSpawnObjectComponentData & SpawnObjectComponent.SpawnObjectComponent

export type ClientSpawnObjectComponentDefinition = { } & SpawnObjectComponent.SpawnObjectComponentDefinition

function _M.new(gameobject, definition): ClientSpawnObjectComponent
	local self = SpawnObjectComponent.new(gameobject, definition) :: ClientSpawnObjectComponent
	return setmetatable(self, _M)
end

return _M
