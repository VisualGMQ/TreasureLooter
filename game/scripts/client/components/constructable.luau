local ConstructableComponent = require("@common/components/constructable")

local _M = {}
_M.__index = _M
setmetatable(_M, ConstructableComponent)

type ClientConstructableComponentData = {}

export type ClientConstructableComponent = typeof(_M) & ClientConstructableComponentData & ConstructableComponent.ConstructableComponent

export type ClientConstructableComponentDefinition = {} & ConstructableComponent.ConstructableComponentDefinition

function _M.new(gameobject, definition): ClientConstructableComponent
	local self = ConstructableComponent.new(gameobject, definition) :: ClientConstructableComponent
	return setmetatable(self, _M)
end

return _M
