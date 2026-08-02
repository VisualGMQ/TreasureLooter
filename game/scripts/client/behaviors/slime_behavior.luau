local ClientLogicComponent = require("@client/components/logic")
local Component = require("@common/components/component")

local _M = {}
_M.__index = _M
setmetatable(_M, ClientLogicComponent)

type SlimeLogicData = {
    m_change_move_dir_timer:Timer,
    m_move_dir:Vec2,
    m_move_cooldown:number,
}

export type SlimeLogic = typeof(_M) & SlimeLogicData & ClientLogicComponent.ClientLogicComponent

function _M.new(gameobject):SlimeLogic
    local self = Component.new(gameobject) :: SlimeLogic
    return setmetatable(self, _M)
end

function _M.OnInit(self:SlimeLogic)
    local ctx: ClientContext = TL_Client.GetContext()

    self.m_move_dir = TL_Common.Vec2.ZERO
    self.m_move_cooldown = 2

    self.m_change_move_dir_timer = ctx:GetTimerManager():Create(3, TL_Schema.TimerEventType.Unknown, 0)
    self.m_change_move_dir_timer:SetTimerListener(function(event)
        self.m_move_dir = TL_Common.Vec2.ZERO
    end)
    self.m_change_move_dir_timer:Start()
end

function _M.OnQuit(self:SlimeLogic)
    self:cleanupTimer()
end

function _M.OnLogicTerminate(self:SlimeLogic)
    self:cleanupTimer()
end

function _M.cleanupTimer(self:SlimeLogic)
    local ctx: ClientContext = TL_Client.GetContext()
    if self.m_change_move_dir_timer then
        ctx:GetTimerManager():Remove(self.m_change_move_dir_timer:GetID())
    end
end

function _M.OnUpdate(self:SlimeLogic, elapse_time:TimeType)
    local go = self:GetGameObject()

    if go.m_buff_receive_component then
        go.m_buff_receive_component:Update(elapse_time)
    end

    if go.m_buff_receive_component and go.m_buff_receive_component:IsFrozen() then
        if go.m_move_component then
            go.m_move_component:StopMove()
        end
        return
    end

    if go.m_hp_component:IsDead() then
        if go.m_move_component then
            go.m_move_component:StopMove()
        end
        return
    end

    if self.m_move_dir == TL_Common.Vec2.ZERO then
        self.m_move_cooldown -= elapse_time
    end
    go.m_move_component:SetDir(self.m_move_dir)
    go.m_move_component:Update(elapse_time)

    if self.m_move_cooldown < 0 then
        self.m_move_cooldown = 2
        local x = math.random() - 0.5
        local y = math.random() - 0.5
        self.m_move_dir = TL_Common.Vec2(x, y):Normalize()
        self.m_change_move_dir_timer:Start()
    end
end

return _M
