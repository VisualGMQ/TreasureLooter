local ConstructableComponent = require("@common/components/constructable")

local _M = {}
_M.__index = _M
setmetatable(_M, ConstructableComponent)

type ClientConstructableComponentData = {
	_growth_timer: Timer,
}

export type ClientConstructableComponent = typeof(_M) & ClientConstructableComponentData & ConstructableComponent.ConstructableComponent

export type ClientConstructableComponentDefinition = {} & ConstructableComponent.ConstructableComponentDefinition

function _M.new(gameobject, definition): ClientConstructableComponent
	local self = ConstructableComponent.new(gameobject, definition) :: ClientConstructableComponent

	local ctx = TL_Client.GetContext()
	self._growth_timer = ctx:GetTimerManager():Create(1, TL_Schema.TimerEventType.Unknown, 0)
	self._growth_timer:SetTimerListener(function(event)
		self:onGrowthTimerTick()
	end)

	return setmetatable(self, _M)
end

function _M.onGrowthTimerTick(self: ClientConstructableComponent)
	if self._index == 0 or self._index > #self._phases then
		self._growth_timer:Stop()
		return
	end

	local phase = self._phases[self._index]
	if not phase or phase.m_auto_grow_factor <= 0 then
		self._growth_timer:Stop()
		return
	end

	self:AddGrowth(phase.m_auto_grow_factor)

	if self._index > #self._phases then
		self._growth_timer:Stop()
		return
	end

	local next_phase = self._phases[self._index]
	if next_phase and next_phase.m_auto_grow_factor > 0 then
		self._growth_timer:Start()
	end
end

function _M.StepToNextPhase(self: ClientConstructableComponent)
	ConstructableComponent.StepToNextPhase(self)

	if self._index > 0 and self._index <= #self._phases then
		local phase = self._phases[self._index]
		if phase and phase.m_auto_grow_factor > 0 then
			self._growth_timer:Start()
		else
			self._growth_timer:Stop()
		end
	end
end

function _M.ResetToPhase(self: ClientConstructableComponent, target_index: number)
	ConstructableComponent.ResetToPhase(self, target_index)

	if self._index > 0 and self._index <= #self._phases then
		local phase = self._phases[self._index]
		if phase and phase.m_auto_grow_factor > 0 then
			self._growth_timer:Start()
		else
			self._growth_timer:Stop()
		end
	end
end

function _M.OnQuit(self: ClientConstructableComponent)
	if self._growth_timer then
		local ctx = TL_Client.GetContext()
		ctx:GetTimerManager():Remove(self._growth_timer:GetID())
	end
end

return _M
