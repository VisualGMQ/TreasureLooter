local ObjectDefinitionTable = require("common.object_def_table")

---@class World
---@field m_land_entity LogicEntity
---@field m_object_definitions ObjectDefinitionTable
---@field m_buff_appliers any
local _M = {}
_M.__index = _M

---@type World?
local g_world = nil

---@return World
function _M.new()
    local self = setmetatable({}, _M)
    self.m_land_entity = TL_Common.null_entity
    return self
end

---@param world World
function _M.SetInst(world)
    g_world = world
end

---@return World
function _M.GetInst()
    assert(g_world, "world is not initialized")
    return g_world
end

return _M
