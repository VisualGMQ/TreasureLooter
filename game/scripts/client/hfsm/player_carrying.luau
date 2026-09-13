local HFSMUtil = require("@client/hfsm/hfsm_util")
local States = require("@client/hfsm/player_states")

local _M = {}
_M.__index = _M

export type PlayerCarryingNode = typeof(_M) & {
    _entity: Entity,
}

function _M.new(entity: Entity): PlayerCarryingNode
    local self = {} :: PlayerCarryingNode
    self._entity = entity
    return setmetatable(self, _M)
end

function _M.OnUpdate(self: PlayerCarryingNode)
    local ctx = TL_Client.GetContext()
    local go = HFSMUtil.GetGameObject(self._entity)
    if not go then
        return
    end

    local raise_up = go.m_raise_up_component
    if not raise_up or not raise_up:IsHolding() then
        HFSMUtil.ChangeState(self._entity, States.FreeHand)
        return
    end

    local input_manager = ctx:GetInputManager()
    local interact_action = input_manager:GetAction("Interact")
    if interact_action:IsPressed(0) then
        local obj = go.m_interact_component and go.m_interact_component:GetInteractObject()
        if obj then
            HFSMUtil.ChangeState(self._entity, States.PickingUp)
        else
            HFSMUtil.ChangeState(self._entity, States.PuttingDown)
        end
    end
end

function _M.OnQuit(self: PlayerCarryingNode)
end

return _M
