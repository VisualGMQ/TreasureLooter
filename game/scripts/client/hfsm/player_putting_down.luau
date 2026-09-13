local ClientAttackComponent = require("@client/components/attack")
local HFSMUtil = require("@client/hfsm/hfsm_util")
local States = require("@client/hfsm/player_states")

local k_put_down_distance = 20

local _M = {}
_M.__index = _M

export type PlayerPuttingDownNode = typeof(_M) & {
    _entity: Entity,
}

function _M.new(entity: Entity): PlayerPuttingDownNode
    local self = {} :: PlayerPuttingDownNode
    self._entity = entity
    return setmetatable(self, _M)
end

function _M.OnUpdate(self: PlayerPuttingDownNode)
    local go = HFSMUtil.GetGameObject(self._entity)
    if go and go.m_raise_up_component then
        local facing = go.m_move_component:GetLastMoveDirection()
        if facing:LengthSquared() == 0 then
            facing = ClientAttackComponent.k_init_forward_dir
        end
        local drop_position = go.m_transform:GetGlobalPosition() + facing * k_put_down_distance
        go.m_raise_up_component:PutDown(drop_position)
    end

    if go and go.m_raise_up_component and go.m_raise_up_component:IsHolding() then
        HFSMUtil.ChangeState(self._entity, States.Carrying)
    else
        HFSMUtil.ChangeState(self._entity, States.FreeHand)
    end
end

function _M.OnInit(self: PlayerPuttingDownNode)
end

function _M.OnQuit(self: PlayerPuttingDownNode)
end

return _M
