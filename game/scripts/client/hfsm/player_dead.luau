local HFSMUtil = require("@client/hfsm/hfsm_util")

local _M = {}
_M.__index = _M

export type PlayerDeadNode = typeof(_M) & {
    _entity: Entity,
}

function _M.new(entity: Entity): PlayerDeadNode
    local self = {} :: PlayerDeadNode
    self._entity = entity
    return setmetatable(self, _M)
end

function _M.OnInit(self: PlayerDeadNode)
    local go = HFSMUtil.GetGameObject(self._entity)
    if go and go.m_move_component then
        go.m_move_component:StopMove()
    end
end

function _M.OnUpdate(self: PlayerDeadNode)
end

function _M.OnQuit(self: PlayerDeadNode)
end

return _M
