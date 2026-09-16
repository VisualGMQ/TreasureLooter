local HFSMUtil = require("client.hfsm.hfsm_util")
local States = require("client.hfsm.player_states")

---@class PlayerFrozenNode
---@field _entity LogicEntity
local _M = {}
_M.__index = _M

---@param entity LogicEntity
---@return PlayerFrozenNode
function _M.new(entity)
    local self = setmetatable({}, _M)
    self._entity = entity
    return self
end

function _M:OnInit()
    local go = HFSMUtil.GetGameObject(self._entity)
    if go and go.m_move_component then
        go.m_move_component:StopMove()
    end
end

function _M:OnUpdate()
    local go = HFSMUtil.GetGameObject(self._entity)
    if not go then
        return
    end

    if not (go.m_buff_receive_component and go.m_buff_receive_component:IsFrozen()) then
        if go.m_raise_up_component and go.m_raise_up_component:IsHolding() then
            HFSMUtil.ChangeState(self._entity, States.Carrying)
        else
            HFSMUtil.ChangeState(self._entity, States.FreeHand)
        end
    end
end

function _M:OnQuit()
end

return _M
