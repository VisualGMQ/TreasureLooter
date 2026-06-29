local GameEntry = require("@common/game_entry")
local ClientCreation = require("@client/creation")
local DebugPanel = require("@client/debug_panel")
local ClientWorld = require("@client/world")

local ClientGameEntry = {}
ClientGameEntry.__index = ClientGameEntry
setmetatable(ClientGameEntry, { __index = GameEntry })

type ClientGameEntryData = {}
export type ClientGameEntry = typeof(ClientGameEntry) & ClientGameEntryData & GameEntry.GameEntry

function ClientGameEntry.new(entity: Entity) : ClientGameEntry
    local self = setmetatable(GameEntry.new(entity, ClientCreation), ClientGameEntry)
    return self
end

function ClientGameEntry.OnInit(self:ClientGameEntry)
    ClientWorld.SetInst(ClientWorld.new())
    GameEntry.OnInit(self)
end

function ClientGameEntry.OnRender(self:ClientGameEntry)
    DebugPanel.ShowDebugPanel()
end

return ClientGameEntry
