local ServerMoveComponent = require("@server/components/move")
local GameObject = require("@common/gameobject")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = GameObject })

type ServerGameObjectData = {
    m_move_component:ServerMoveComponent.ServerMoveComponent,
}
export type ServerGameObject = typeof(_M) & ServerGameObjectData & GameObject.GameObject

export type ServerGameObjectDefinition = {
    m_did:DID,
    m_move_component_definition:ServerMoveComponent.ServerMoveComponentDefinition?,
}

function _M.new(entity:Entity, definition:ServerGameObjectDefinition):ServerGameObject
    local self = GameObject.new(entity, definition.m_did) :: ServerGameObject
    if definition.m_move_component_definition then
        self.m_move_component = ServerMoveComponent.new(self, definition.m_move_component_definition)
    end
    return setmetatable(self, _M)
end

function _M.OnUpdate(self:ServerGameObject, elapse_time:TimeType)
    if self.m_move_component then
        self.m_move_component:Update(elapse_time)
    end
end

function _M.OnQuit(self:ServerGameObject)
    if self.m_move_component then
        self.m_move_component:OnQuit()
    end
end

return _M
