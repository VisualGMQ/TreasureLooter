local ClientGameObjectBehavior = require("@client/gameobject_behavior")

local _M = {}
_M.__index = _M
setmetatable(_M, ClientGameObjectBehavior)

type ChoppableTreeLogicData = {
}

export type ChoppableTreeLogic = typeof(_M) & ChoppableTreeLogicData & ClientGameObjectBehavior.ClientGameObjectBehavior

function _M.new(gameobject):ChoppableTreeLogic
	local self = ClientGameObjectBehavior.new(gameobject) :: ChoppableTreeLogic
	return setmetatable(self, _M)
end

function _M.OnInit(self:ChoppableTreeLogic)
end

function _M.OnUpdate(self:ChoppableTreeLogic, elapse_time:TimeType)
    ClientGameObjectBehavior.OnUpdate(self, elapse_time)
end

function _M.OnQuit(self:ChoppableTreeLogic)
end

return _M
