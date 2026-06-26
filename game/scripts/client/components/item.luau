local ItemComponent = require("@common/components/item")

local _M = {}
_M.__index = _M
setmetatable(_M, ItemComponent)

type ClientItemComponentData = { }

export type ClientItemComponent = typeof(_M) & ClientItemComponentData & ItemComponent.ItemComponent

export type ClientItemComponentDefinition = { } & ItemComponent.ItemComponentDefinition

function _M.new(gameobject, definition): ClientItemComponent
    local self = ItemComponent.new(gameobject, definition)::ClientItemComponent
    return setmetatable(self, _M)
end

return _M
