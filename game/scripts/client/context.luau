local TL_Common = require("@type_hints/tl_common_types")



export type LuaContext = {
    m_player_layer_entity: TL_Common.Entity,
}

local _M = {}

local function new()
    local self = {} :: LuaContext
    self.m_player_layer_entity = TL_Common.null_entity
    return setmetatable(self, _M)
end

local context = new()

return context


