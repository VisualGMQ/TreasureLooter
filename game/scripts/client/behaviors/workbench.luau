local ClientGameObjectBehavior = require("@client/gameobject_behavior")

local _M = {}
_M.__index = _M
setmetatable(_M, ClientGameObjectBehavior)

type WorkbenchLogicData = {}

export type WorkbenchLogic = typeof(_M) & WorkbenchLogicData & ClientGameObjectBehavior.ClientGameObjectBehavior

function _M.new(gameobject):WorkbenchLogic
	local self = ClientGameObjectBehavior.new(gameobject) :: WorkbenchLogic
	return setmetatable(self, _M)
end

function _M.OnInit(self:WorkbenchLogic)
end

function _M.OnUpdate(self:WorkbenchLogic, elapse_time:TimeType)
	ClientGameObjectBehavior.OnUpdate(self, elapse_time)
end

return _M
