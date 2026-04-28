local TL_Common = require("@type_hints/tl_common_types")
local TL_Client = require("@type_hints/tl_client_types")
local TL_Schema = require("@type_hints/tl_schema_types")


local ScriptBehavior = require("@client/script_behavior")
local Common = require("@client/common")
local Character = require("@client/character")

local Slime = {}
Slime.__index = Slime
setmetatable(Slime, { __index = ScriptBehavior })

type SlimeData = {
    m_character: Character.Character,

	m_direction : Common.Direction,

    m_move_cooldown: number,
    m_move_dir: TL_Common.Vec2,

}

export type Slime = typeof(setmetatable({}::SlimeData, Slime)) & ScriptBehavior.ScriptBehavior

function Slime.new(entity: number): Slime
	local self = ScriptBehavior.new(entity)
	return setmetatable(self, Slime)
end

function Slime.OnInit(self: Slime)
	local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()

	self.m_direction = Common.Direction.Down

    self.m_move_dir = TL_Common.Vec2.ZERO

	self.m_change_move_dir_timer = ctx:GetTimerManager():Create(3, TL_Schema.TimerEventType.Unknown, -1)
	self.m_change_move_dir_timer:Start()

    self.m_character:SubscribeEvents(self)
end

function Slime._init(self:Slime, character: Character.Character)
    self.m_character = character
end

function Slime.OnUpdate(self: Slime, elapse_time: number)
    self.m_character:Move(self.m_move_dir, elapse_time)
end

function Slime.OnQuit(self: Slime)
    self.m_character:OnQuit()

	local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()
	if self.m_change_move_dir_timer then
		ctx:GetTimerManager():Remove(self.m_change_move_dir_timer:GetID())
	end
end

function Slime.OnTimerEvent(self: Slime, event: TL_Common.TimerEvent)
	local timer_id = event:GetID()
	if timer_id == self.m_change_move_dir_timer:GetID() then
		local x = math.random() - 0.5
		local y = math.random() - 0.5
		self.m_move_dir = TL_Common.Vec2(x, y):Normalize()
	end

    self.m_character:OnTimerEvent(event)
end

function Slime.OnTimerStopEvent(self: Slime, event: TL_Common.TimerStopEvent)
    self.m_character:OnTimerStopEvent(event)
end

function Slime.OnTriggerEnterEvent(self: Slime, event: TL_Common.TriggerEnterEvent)
	local dst_entity = event:GetOverlapResult().m_dst_entity
	if dst_entity == TL_Common.null_entity or dst_entity == self:GetEntity() then
		return
	end

    self.m_character:OnTriggerEnterEvent(event)
end

return Slime


