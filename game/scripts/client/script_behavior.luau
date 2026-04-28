local TL_Common = require("@type_hints/tl_common_types")
local TL_Client = require("@type_hints/tl_client_types")


local ScriptBehavior = {}
ScriptBehavior.__index = ScriptBehavior

type ScriptBehaviorData = {
	_entity: TL_Common.Entity,
}

export type ScriptBehavior = typeof(setmetatable({}::ScriptBehaviorData, ScriptBehavior))

function ScriptBehavior.new(entity: number): ScriptBehavior
	local self = {}::ScriptBehaviorData

	self._entity = entity

	return setmetatable(self, ScriptBehavior)
end

function ScriptBehavior.GetEntity(self: ScriptBehavior): TL_Common.Entity
	return self._entity
end

function ScriptBehavior.SubscribeEvent(self: ScriptBehavior, eventName: string)
	local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()
	ctx:GetScriptManager():SubscribeEvent(self:GetEntity(), eventName)
end

function ScriptBehavior.SubscribeTriggerEnterEvent(self: ScriptBehavior)
	self:SubscribeEvent("TriggerEnterEvent")
end

function ScriptBehavior.SubscribeTriggerLeaveEvent(self: ScriptBehavior)
	self:SubscribeEvent("TriggerLeaveEvent")
end

function ScriptBehavior.SubscribeTriggerTouchEvent(self: ScriptBehavior)
	self:SubscribeEvent("TriggerTouchEvent")
end

function ScriptBehavior.SubscribeUIMouseHoverEvent(self: ScriptBehavior)
	self:SubscribeEvent("UIMouseHoverEvent")
end

function ScriptBehavior.SubscribeUIMouseDownEvent(self: ScriptBehavior)
	self:SubscribeEvent("UIMouseDownEvent")
end

function ScriptBehavior.SubscribeUIMouseUpEvent(self: ScriptBehavior)
	self:SubscribeEvent("UIMouseUpEvent")
end

function ScriptBehavior.SubscribeUIMouseClickedEvent(self: ScriptBehavior)
	self:SubscribeEvent("UIMouseClickedEvent")
end

function ScriptBehavior.SubscribeUICheckToggledEvent(self: ScriptBehavior)
	self:SubscribeEvent("UICheckToggledEvent")
end

function ScriptBehavior.SubscribeUIDragEvent(self: ScriptBehavior)
	self:SubscribeEvent("UIDragEvent")
end

function ScriptBehavior.SubscribeTimerEvent(self: ScriptBehavior)
	self:SubscribeEvent("TimerEvent")
end

function ScriptBehavior.SubscribeTimerStopEvent(self: ScriptBehavior)
	self:SubscribeEvent("TimerStopEvent")
end

function ScriptBehavior.SubscribeDebugEvent(self: ScriptBehavior)
	self:SubscribeEvent("DebugEvent")
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


