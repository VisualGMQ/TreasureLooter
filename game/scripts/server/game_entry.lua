local GameEntry = require("common.game_entry")
local ServerCreation = require("server.creation")
local ServerWorld = require("server.world")
local Net = require("common.net")

local k_max_peers = 32
local k_player_script = TL_Common.Path("scripts/server/behaviors/net_player_behavior.lua")
local k_player_spawn_point = "player_spawn_point"

---@class ServerGameEntryData

---@class ServerGameEntry : GameEntry
local ServerGameEntry = {}
ServerGameEntry.__index = ServerGameEntry
setmetatable(ServerGameEntry, { __index = GameEntry })

---@param entity Entity
---@return ServerGameEntry
function ServerGameEntry.new(entity)
    local self = setmetatable(GameEntry.new(entity, ServerCreation), ServerGameEntry)
    ---@cast self ServerGameEntry
    return self
end

function ServerGameEntry:OnInit()
    ServerWorld.SetInst(ServerWorld.new())

    GameEntry.OnInit(self)

    local ctx = TL_Server.GetContext()
    ctx:NetListen(TL_Common.NetAddress(Net.ip, Net.port), k_max_peers)
    ctx:Log("server listening on ", Net.ip, ":", Net.port)

    local event_system = ctx:GetEventSystem()
    event_system:AddNetMsg_SpawnPlayerEvent(function(id, peer, payload)
        self:onSpawnPlayer(peer, payload)
    end)

    event_system:AddNetMsg_DisconnectEvent(function(id, peer, payload)
        self:onNetDisconnect(peer, payload)
    end)
end

---@param peer UDPPeer
---@param payload ProtoSpawnPlayer
function ServerGameEntry:onSpawnPlayer(peer, payload)
    local ctx = TL_Server.GetContext()
    local spawn = payload:to_schema()
    local did = spawn.m_did

    local spawn_point = self.m_spawn_points[k_player_spawn_point]
    if not spawn_point then
        ctx:Log("SpawnPlayer: can't find spawn point ", k_player_spawn_point)
        return
    end

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return
    end

    local spawn_info = TL_Schema.ObjectSpawnDefinition()
    spawn_info.m_did = did
    spawn_info.m_server_script = k_player_script
    spawn_info.m_spawn_point_name = k_player_spawn_point

    local entity, go = ServerCreation.CreateCharacter(ServerCreation, scene, spawn_info,
                            spawn_point.m_position, self.m_object_definitions)

    local root_entity = scene:GetRootEntity()
    local root_relationship = ctx:GetRelationshipManager():Get(root_entity)
    root_relationship:AddChild(entity)
    ctx:Log("server spawned player by did ", did)

    ---@type ServerWorld
    local world = ServerWorld.GetInst()
    world:AddPeer(peer, go)
    ctx:Log("peer ", peer:GetID(), " spawned")
end

---@param peer UDPPeer
---@param payload ProtoDisconnect
function ServerGameEntry:onNetDisconnect(peer, payload)
    ---@type ServerWorld
    local world = ServerWorld.GetInst()
    world:RemovePeer(peer)

    TL_Server.GetContext():Log("peer ", peer:GetID(), " disconnected")
end

return ServerGameEntry
