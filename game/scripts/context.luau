local TL = require("@game/tl_types")

export type LuaContext = {
    m_player_layer_entity: TL.Entity,
}

local _M = {}

local function new()
    local self = {} :: LuaContext
    print("TL = ", TL)
    self.m_player_layer_entity = TL.null_entity
    return setmetatable(self, _M)
end

local context = new()

return context
