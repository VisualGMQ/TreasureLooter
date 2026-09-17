local GameEntry = require("common.game_entry")
local ServerCreation = require("server.creation")
local ServerWorld = require("server.world")

local k_max_peers = 32

---@class ServerGameEntryData

---@class ServerGameEntry : GameEntry
local ServerGameEntry = {}
ServerGameEntry.__index = ServerGameEntry
setmetatable(ServerGameEntry, { __index = GameEntry })

---@param entity LogicEntity
---@return ServerGameEntry
function ServerGameEntry.new(entity)
    local self = setmetatable(GameEntry.new(entity, ServerCreation), ServerGameEntry)
    ---@cast self ServerGameEntry
    return self
end

function ServerGameEntry:OnInit()
    local world = ServerWorld.new()
    ServerWorld.SetInst(world)

    GameEntry.OnInit(self)

    local ctx = TL_Server.GetContext()
    local config = ctx:GetConfig()
    ctx:NetListen(TL_Common.NetAddress(config.m_listen_ip, config.m_listen_port), k_max_peers)
    ctx:Log("server listening on ", config.m_listen_ip, ":", config.m_listen_port)
    world:RegisterNetEventHandler()
end

--- The global script is updated every frame: it drives the world's post-win
--- countdown (there is no per-world update callback).
---@param elapse_time TimeType
function ServerGameEntry:OnUpdate(elapse_time)
    local world = ServerWorld.GetInst()
    ---@cast world ServerWorld
    world:Update(elapse_time)
end

return ServerGameEntry
