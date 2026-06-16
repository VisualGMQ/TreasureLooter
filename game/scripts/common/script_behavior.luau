
local ScriptBehavior = {}
ScriptBehavior.__index = ScriptBehavior

type ScriptBehaviorData = {
	_entity: Entity,
}

export type ScriptBehavior = typeof(ScriptBehavior) & ScriptBehaviorData

function ScriptBehavior.new(entity: number): ScriptBehavior
	local self = {}::ScriptBehaviorData

	self._entity = entity

	return setmetatable(self, ScriptBehavior)
end

function ScriptBehavior.GetEntity(self: ScriptBehavior): Entity
	return self._entity
end

function ScriptBehavior.OnInit(self)
end

function ScriptBehavior.OnQuit(self)
end

function ScriptBehavior.OnUpdate(self, elapse_time: number)
end

function ScriptBehavior.OnRender(self)
end

return ScriptBehavior


