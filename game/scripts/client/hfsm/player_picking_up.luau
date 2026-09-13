local HFSMUtil = require("@client/hfsm/hfsm_util")
local States = require("@client/hfsm/player_states")

local _M = {}
_M.__index = _M

export type PlayerPickingUpNode = typeof(_M) & {
    _entity: Entity,
}

function _M.new(entity: Entity): PlayerPickingUpNode
    local self = {} :: PlayerPickingUpNode
    self._entity = entity
    return setmetatable(self, _M)
end

function _M.OnUpdate(self: PlayerPickingUpNode)
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

function _M.OnInit(self: PlayerPickingUpNode)
end

function _M.OnQuit(self: PlayerPickingUpNode)
end

return _M
