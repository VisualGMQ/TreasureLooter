local World = require("@common/world")

local _M = {}
_M.__index = _M
setmetatable(_M, World)

type ClientWorldData = {
    m_player_hint:Entity,
}

export type ClientWorld = typeof(_M) & ClientWorldData & World.World

function _M.new():ClientWorld
    local self = World.new()::ClientWorld
    self.m_player_hint = TL_Common.null_entity
    return setmetatable(self, _M)
end

return _M
