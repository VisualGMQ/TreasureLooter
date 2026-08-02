local LogicComponent = require("@common/components/logic")

local _M = {}
_M.__index = _M
setmetatable(_M, LogicComponent)

type ClientLogicComponentData = {}

export type ClientLogicComponent = typeof(_M) & ClientLogicComponentData & LogicComponent.LogicComponent

function _M.OnRender(self: ClientLogicComponent)
end

return _M
