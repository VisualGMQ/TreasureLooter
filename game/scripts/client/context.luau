local TL = require("@client/tl_types")
local TLC = TL.Common
local TLI = TL.Client
local TLS = TL.Server


export type LuaContext = {
    m_player_layer_entity: TLC.Entity,
}

local _M = {}

local function new()
    local self = {} :: LuaContext
    print("TL = ", TL)
    self.m_player_layer_entity = TLC.null_entity
    return setmetatable(self, _M)
end

local context = new()

return context


