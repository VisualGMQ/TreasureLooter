local HFSMUtil = require("client.hfsm.hfsm_util")

---@class PlayerDeadNode
---@field _entity Entity
local _M = {}
_M.__index = _M

---@param entity Entity
---@return PlayerDeadNode
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
end

function _M:OnQuit()
end

return _M
