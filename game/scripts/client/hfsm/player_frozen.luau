local HFSMUtil = require("@client/hfsm/hfsm_util")

local k_alive_state_id = 0
local k_dead_state_id = 2

local _M = {}
_M.__index = _M

export type PlayerFrozenNode = typeof(_M) & {
    _entity: Entity,
}

function _M.new(entity: Entity): PlayerFrozenNode
    local self = {} :: PlayerFrozenNode
    self._entity = entity
    return setmetatable(self, _M)
end

function _M.OnInit(self: PlayerFrozenNode)
    local go = HFSMUtil.GetGameObject(self._entity)
    if go and go.m_move_component then
        go.m_move_component:StopMove()
    end
end

function _M.OnUpdate(self: PlayerFrozenNode)
    local elapse_time = HFSMUtil.GetElapseTime()
    local go = HFSMUtil.GetGameObject(self._entity)

    if not go then
        return
    end

    if go.m_buff_receive_component then
        go.m_buff_receive_component:Update(elapse_time)
    end

    if not (go.m_buff_receive_component and go.m_buff_receive_component:IsFrozen()) then
        if go.m_hp_component:IsDead() then
            HFSMUtil.ChangeState(self._entity, k_dead_state_id)
            return
        end
        HFSMUtil.ChangeState(self._entity, k_alive_state_id)
    end
end

function _M.OnQuit(self: PlayerFrozenNode)
end

return _M
