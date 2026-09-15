local HFSMUtil = require("client.hfsm.hfsm_util")
local States = require("client.hfsm.player_states")

---@class PlayerPickingUpNode
---@field _entity Entity
local _M = {}
_M.__index = _M

---@param entity Entity
---@return PlayerPickingUpNode
function _M.new(entity)
    local self = setmetatable({}, _M)
    self._entity = entity
    return self
end

function _M:OnUpdate()
    local go = HFSMUtil.GetGameObject(self._entity)
    if go and go.m_interact_component and go.m_raise_up_component then
        local obj = go.m_interact_component:GetInteractObject()
        if obj then
            go.m_raise_up_component:TryRaiseUp(obj)
        end
    end

    if go and go.m_raise_up_component and go.m_raise_up_component:IsHolding() then
        HFSMUtil.ChangeState(self._entity, States.Carrying)
    else
        HFSMUtil.ChangeState(self._entity, States.FreeHand)
    end
end

function _M:OnInit()
end

function _M:OnQuit()
end

return _M
