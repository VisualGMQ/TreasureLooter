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

---@param entity LogicEntity
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
    event_system:AddNetMsg_SpawnPlayerRequestEvent(function(id, peer, payload)
        self:onSpawnPlayerRequest(peer, payload)
    end)

    event_system:AddNetMsg_DisconnectEvent(function(id, peer, payload)
        self:onNetDisconnect(peer, payload)
    end)
end

---@param peer UDPPeer
---@param payload ProtoSpawnPlayerRequest
function ServerGameEntry:onSpawnPlayerRequest(peer, payload)
    local ctx = TL_Server.GetContext()
    local did = payload:m_did()

    local spawn_point = self.m_spawn_points[k_player_spawn_point]
    if not spawn_point then
        ctx:Log("SpawnPlayerRequest: can't find spawn point ", k_player_spawn_point)
        return
    end

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return
    end

    local world = ServerWorld.GetInst()
    ---@cast world ServerWorld
    local net_id = peer:GetID()
    local position = spawn_point.m_position

    local spawn_info = TL_Schema.ObjectSpawnDefinition()
    spawn_info.m_did = did
    spawn_info.m_server_script = k_player_script
    spawn_info.m_spawn_point_name = k_player_spawn_point

    local entity, go = ServerCreation.CreateCharacter(ServerCreation, scene, spawn_info,
        position, net_id, self.m_object_definitions)

    local root_entity = scene:GetRootEntity()
    local root_relationship = ctx:GetRelationshipManager():Get(root_entity)
    if root_relationship then
        root_relationship:AddChild(entity)
    end

    world:AddPeer(peer, go)
    ctx:Log("server spawned player by did ", did, " net_id ", net_id)

    local net_position = TL_Proto.NetVec2()
    net_position:set_m_x(position.x)
    net_position:set_m_y(position.y)

    local reply = TL_Proto.SpawnPlayerReply()
    reply:set_m_did(did)
    reply:set_m_position(net_position)

    local net_msg = TL_Proto.NetMsg()
    net_msg:set_m_spawn_player_reply(reply)

    local host = ctx:GetNetHost()
    if host then
        host:Send(peer, net_msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
    end
end

---@param peer UDPPeer
---@param payload ProtoDisconnect
function ServerGameEntry:onNetDisconnect(peer, payload)
    local world = ServerWorld.GetInst()
    ---@cast world ServerWorld
    world:RemovePeer(peer)

    TL_Server.GetContext():Log("peer ", peer:GetID(), " disconnected")
end

return ServerGameEntry
