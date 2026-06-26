local Component = require("@common/components/component")

local _M = {}
_M.__index = _M
setmetatable(_M, Component)

type LogicComponentData = {}

export type LogicComponent = typeof(_M) & LogicComponentData & Component.Component

function _M.OnInit(self: LogicComponent)
end

function _M.OnUpdate(self: LogicComponent, elapse_time: TimeType)
end

function _M.OnQuit(self: LogicComponent)
end

function _M.OnLogicTerminate(self: LogicComponent)
end

return _M
