local GameEntry = require("@common/game_entry")
local ServerCreation = require("@server/creation")

local ServerGameEntry = {}
ServerGameEntry.__index = ServerGameEntry
setmetatable(ServerGameEntry, { __index = GameEntry })

type ServerGameEntryData = {}
export type ServerGameEntry = typeof(ServerGameEntry) & ServerGameEntryData & GameEntry.GameEntry

function ServerGameEntry.new(entity: Entity) : ServerGameEntry
    local self = setmetatable(GameEntry.new(entity, ServerCreation), ServerGameEntry)
    return self
end

return ServerGameEntry
