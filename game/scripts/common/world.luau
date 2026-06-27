local _M = {}
_M.__index = _M

type WorldData = {
    m_land_entity:Entity,
}

export type World = typeof(_M) & WorldData

local g_world:World? = nil

function _M.new():World
    local self = {}::WorldData
    self.m_land_entity = TL_Common.null_entity
    return setmetatable(self, _M)
end

function _M.SetInst(world:World)
    g_world = world
end

function _M.GetInst():World
    assert(g_world, "world is not initialized")
    return g_world :: World
end

return _M
