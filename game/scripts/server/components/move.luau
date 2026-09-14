local MoveComponent = require("@common/components/move")
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = MoveComponent })

export type ServerMoveComponent = typeof(_M) & MoveComponent.MoveComponent

export type ServerMoveComponentDefinition = {
    m_cct: CharacterController?,
    m_speed: number,
}

function _M.new(gameobject, definition: ServerMoveComponentDefinition): ServerMoveComponent
    local self = MoveComponent.new(gameobject, definition.m_cct, definition.m_speed) :: ServerMoveComponent
    return setmetatable(self, _M)
end

return _M
