local HFSMUtil = require("client.hfsm.hfsm_util")
local States = require("client.hfsm.player_states")
local ClientWorld = require("client.world")

--- The state a freshly spawned player starts in.
---@param go any
---@return integer
local function getInitialState(go)
    if go and go.m_raise_up_component and go.m_raise_up_component:IsHolding() then
        return States.Carrying
    end
    return States.FreeHand
end

---@class PlayerRootNode
---@field _entity LogicEntity
---@field private _parked_by_lock boolean
local _M = {}
_M.__index = _M

---@param entity LogicEntity
---@return PlayerRootNode
function _M.new(entity)
    local self = setmetatable({}, _M)
    self._entity = entity
    self._parked_by_lock = false
    return self
end

function _M:OnInit()
    local go = HFSMUtil.GetGameObject(self._entity)
    HFSMUtil.ChangeState(self._entity, getInitialState(go))
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

    -- A `Win` freezes the local player (see ClientWorld:IsGameOver): park it in
    -- the Dead state, whose node reads no input, so the HFSM stops moving,
    -- attacking and interacting.
    local world = ClientWorld.FindInst()
    ---@cast world ClientWorld|nil
    local game_over = world and world:IsGameOver()

    if (go.m_hp_component and go.m_hp_component:IsDead()) or game_over then
        HFSMUtil.ChangeState(self._entity, States.Dead)
        return
    end

    -- The round has not started yet (the server countdown is still running):
    -- park the player in the Dead state exactly like the game over case, then
    -- restore the normal initial state once the input is unlocked.
    if world and world:IsControlLocked() then
        HFSMUtil.ChangeState(self._entity, States.Dead)
        self._parked_by_lock = true
        return
    end

    if self._parked_by_lock then
        self._parked_by_lock = false
        HFSMUtil.ChangeState(self._entity, getInitialState(go))
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
