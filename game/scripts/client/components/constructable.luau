local ConstructableComponent = require("@common/components/constructable")

local _M = {}
_M.__index = _M
setmetatable(_M, ConstructableComponent)

type ClientConstructableComponentData = {
	_growth_timer: Timer,
}

-- export type ClientConstructableComponent = typeof(setmetatable({}::ClientConstructableComponentData, _M)) & ConstructableComponent.ConstructableComponent
export type ClientConstructableComponent =
	typeof(_M) & ClientConstructableComponentData & ConstructableComponent.ConstructableComponent

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

function _M.onMaterialEnter(self: ClientConstructableComponent, event: TriggerEnterEvent)
	if self._fx_on_accept_material ~= TL_Schema.DID.Invalid then
		local ClientWorld = require("@client/world")
		local pos = self._gameobject.m_transform:GetGlobalPosition()
		pos.y += 10
		ClientWorld.GetInst():AddFX(self._fx_on_accept_material, pos)
	end

	ConstructableComponent.onMaterialEnter(self, event)
end

function _M.updateSprite(self: ClientConstructableComponent)
	if self._index < 1 or self._index > #self._phases then
		return
	end

	local phase = self._phases[self._index]
	if not phase then
		return
	end

	local sprite = TL_Client.GetContext():GetSpriteManager():Get(self._gameobject:GetEntity())
	if sprite then
		sprite.m_image = phase.m_sprite.m_image
		sprite.m_region = phase.m_sprite.m_region
		sprite.m_size = phase.m_sprite.m_size
		sprite.m_anchor = phase.m_sprite.m_anchor
		sprite.m_color = phase.m_sprite.m_color
		sprite.m_flip = phase.m_sprite.m_flip
	end
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
	self:updateSprite()

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
	self:updateSprite()

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
