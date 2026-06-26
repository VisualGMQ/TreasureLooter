local ClientGameObjectBehavior = require("@client/gameobject_behavior")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = ClientGameObjectBehavior })

type SlimeBehaviorData = {
    m_change_move_dir_timer:Timer,
    m_move_dir:Vec2,
    m_move_cooldown:number,
    m_timer_listener:EventListenerID,
}

export type SlimeBehavior = typeof(_M) & SlimeBehaviorData & ClientGameObjectBehavior.ClientGameObjectBehavior

function _M.new(entity:Entity)
    local self = ClientGameObjectBehavior.new(entity)
    return setmetatable(self, _M)
end

function _M.OnInit(self:SlimeBehavior)
    local ctx: ClientContext = TL_Client.GetContext()

    self.m_move_dir = TL_Common.Vec2.ZERO
    self.m_move_cooldown = 2

	self.m_change_move_dir_timer = ctx:GetTimerManager():Create(3, TL_Schema.TimerEventType.Unknown, 0)
	self.m_change_move_dir_timer:Start()

    local es = ctx:GetEventSystem()
    self.m_timer_listener = es:AddTimerEvent(function(id, event)
        if event:GetID() ~= self.m_change_move_dir_timer:GetID() then return end
        self.m_move_dir = TL_Common.Vec2.ZERO
    end)
end

function _M.OnQuit(self:SlimeBehavior)
    local ctx: ClientContext = TL_Client.GetContext()
	if self.m_change_move_dir_timer then
		ctx:GetTimerManager():Remove(self.m_change_move_dir_timer:GetID())
	end
end

function _M.OnUpdate(self:SlimeBehavior, elapse_time:TimeType)
    if self.m_gameobject.m_hp_component:IsDead() then
        if self.m_gameobject.m_move_component then
            self.m_gameobject.m_move_component:StopMove()
        end
        return
    end

    if self.m_move_dir == TL_Common.Vec2.ZERO then
        self.m_move_cooldown -= elapse_time
    end
    self.m_gameobject.m_move_component:SetDir(self.m_move_dir)
    self.m_gameobject.m_move_component:Update(elapse_time)

    if self.m_move_cooldown < 0 then
        self.m_move_cooldown = 2
        local x = math.random() - 0.5
        local y = math.random() - 0.5
        self.m_move_dir = TL_Common.Vec2(x, y):Normalize()
        self.m_change_move_dir_timer:Start()
    end
end

return _M