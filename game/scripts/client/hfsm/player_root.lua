local HFSMUtil = require("client.hfsm.hfsm_util")
local States = require("client.hfsm.player_states")

---@class PlayerRootNode
---@field _entity LogicEntity
local _M = {}
_M.__index = _M

---@param entity LogicEntity
---@return PlayerRootNode
function _M.new(entity)
    local self = setmetatable({}, _M)
    self._entity = entity
    return self
end

function _M:OnInit()
    local go = HFSMUtil.GetGameObject(self._entity)
    local initial = States.FreeHand
    if go and go.m_raise_up_component and go.m_raise_up_component:IsHolding() then
        initial = States.Carrying
    end
    HFSMUtil.ChangeState(self._entity, initial)
end

function _M:OnUpdate()
    local ctx = TL_Client.GetContext()
    local elapse_time = HFSMUtil.GetElapseTime()
    local go = HFSMUtil.GetGameObject(self._entity)
    if not go then
        return
    end

    if go.m_buff_receive_component then
        go.m_buff_receive_component:Update(elapse_time)
    end

    if go.m_hp_component and go.m_hp_component:IsDead() then
        HFSMUtil.ChangeState(self._entity, States.Dead)
        return
    end

    if go.m_buff_receive_component and go.m_buff_receive_component:IsFrozen() then
        HFSMUtil.ChangeState(self._entity, States.Frozen)
        return
    end

    ctx:GetCamera():MoveTo(go.m_transform.m_position)
end

function _M:OnQuit()
end

return _M
