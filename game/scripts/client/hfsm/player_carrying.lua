local HFSMUtil = require("client.hfsm.hfsm_util")
local States = require("client.hfsm.player_states")

---@class PlayerCarryingNode
---@field _entity Entity
local _M = {}
_M.__index = _M

---@param entity Entity
---@return PlayerCarryingNode
function _M.new(entity)
    local self = setmetatable({}, _M)
    self._entity = entity
    return self
end

function _M:OnUpdate()
    local go = HFSMUtil.GetGameObject(self._entity)
    if not go then
        return
    end

    local raise_up = go.m_raise_up_component
    if not raise_up or not raise_up:IsHolding() then
        HFSMUtil.ChangeState(self._entity, States.FreeHand)
        return
    end
end

function _M:OnQuit()
end

return _M
