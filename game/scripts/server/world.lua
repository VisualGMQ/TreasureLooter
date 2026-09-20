local World = require("common.world")
local ServerCreation = require("server.creation")

---@class ServerWorld : World
---@field _replicate_peers table<NetID, ServerGameObject>
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = World })

local k_player_spawn_point = "player_spawn_point"

---@return ServerWorld
function _M.new()
    local self = World.new()
    ---@cast self ServerWorld
    self._replicate_peers = {}
    -- every object the server has spawned, replayed to peers that join later
    self._spawns = {}
    return setmetatable(self, _M)
end

---@param peer UDPPeer
---@param go ServerGameObject
function _M:AddPeer(peer, go)
    self._replicate_peers[peer:GetID()] = go
end

--- Register a spawned object and broadcast it to the connected peers.
---@param entity LogicEntity
---@param spawn_info ObjectSpawnDefinition
---@param net_id number
function _M:AddSpawn(entity, spawn_info, net_id)
    local record = {
        entity = entity,
        did = spawn_info.m_did,
        net_id = net_id or 0,
        client_script = spawn_info.m_client_script and spawn_info.m_client_script:string() or "",
        spawn_on_layer = spawn_info.m_spawn_on_layer or "",
    }
    table.insert(self._spawns, record)
    self:broadcastSpawn(record)
end

---@param record any
---@return ProtoNetMsg
function _M:buildSpawnMsg(record)
    local ctx = TL_Server.GetContext()

    local position = TL_Common.Vec2.ZERO
    local transform = ctx:GetTransformManager():Get(record.entity)
    if transform then
        position = transform:GetGlobalPosition()
    end

    local net_position = TL_Proto.NetVec2()
    net_position:set_m_x(position.x)
    net_position:set_m_y(position.y)

    local spawn = TL_Proto.Spawn()
    spawn:set_m_net_id(record.net_id)
    spawn:set_m_did(record.did)
    spawn:set_m_position(net_position)
    spawn:set_m_client_script(record.client_script)
    spawn:set_m_spawn_on_layer(record.spawn_on_layer)

    local net_msg = TL_Proto.NetMsg()
    net_msg:set_m_spawn(spawn)
    return net_msg
end

---@param record any
function _M:broadcastSpawn(record)
    local host = TL_Server.GetContext():GetNetHost()
    if not host then
        return
    end
    host:Broadcast(self:buildSpawnMsg(record), 0,
                   TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
end

--- @brief remove peer from world and scene
---@param peer UDPPeer
function _M:RemovePeer(peer)
    local id = peer:GetID()
    local go = self._replicate_peers[id]
    if not go then
        return
    end
    local entity = go:GetEntity()
    local ctx = TL_Server.GetContext()
    local relationship_mgr = ctx:GetRelationshipManager()
    local relationship = relationship_mgr:Get(entity)
    if relationship then
        relationship:RemoveFromParent()
    end
    self._replicate_peers[peer:GetID()] = nil

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if scene then
        scene:RemoveEntity(entity)
    end

    self._replicate_peers[id] = nil

    -- Drop the spawn record so this player is not replayed to late joiners.
    for i = #self._spawns, 1, -1 do
        if self._spawns[i].net_id == id then
            table.remove(self._spawns, i)
        end
    end
end

function _M:RegisterNetEventHandler()
    local ctx = TL_Server:GetContext()
    local event_system = ctx:GetEventSystem()
    event_system:AddNetMsg_SpawnPlayerRequestEvent(function(id, peer, payload)
        self:onSpawnPlayerRequest(peer, payload)
    end)
    event_system:AddNetMsg_ConnectEvent(function(id, peer, payload)
        self:onNetConnect(peer)
    end)
    event_system:AddNetMsg_DisconnectEvent(function(id, peer, payload)
        self:onNetDisconnect(peer)
    end)
end

function _M:onNetConnect(peer)
    self:replicateWorldToNewPeer(peer)
end

function _M:onSpawnPlayerRequest(peer, payload)
    self:createPlayer(peer, payload)
end

---@param peer UDPPeer
---@param create_info ProtoSpawnPlayerRequest
function _M:createPlayer(peer, create_info)
    local ctx = TL_Server.GetContext()
    local did = create_info:m_did()

    local spawn_point = self.m_spawn_points[k_player_spawn_point]
    if not spawn_point then
        ctx:Log("SpawnPlayerRequest: can't find spawn point ", k_player_spawn_point)
        return
    end

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return
    end

    local position = spawn_point.m_position

    local spawn_info = TL_Schema.ObjectSpawnDefinition()
    spawn_info.m_did = did
    local player_script = self.m_level_definition and self.m_level_definition.m_server_player_script
    if player_script and not player_script:empty() then
        spawn_info.m_server_script = player_script
    end
    spawn_info.m_spawn_point_name = k_player_spawn_point
    spawn_info.m_spawn_on_layer = "arch"

    local net_id = peer:GetID()
    local entity, go = ServerCreation.CreateCharacter(ServerCreation, scene, spawn_info,
        position, net_id, self.m_object_definitions)

    local root_entity = scene:GetRootEntity()
    local root_relationship = ctx:GetRelationshipManager():Get(root_entity)
    if root_relationship then
        root_relationship:AddChild(entity)
    end

    -- The Spawn message for this player is broadcast by
    -- ServerCreation.CreateCharacter -> World:AddSpawn.
    self:AddPeer(peer, go)
    ctx:Log("server spawned player by did ", did, " net_id ", net_id)
end

---@param peer UDPPeer
function _M:replicateWorldToNewPeer(peer)
    local host = TL_Server.GetContext():GetNetHost()
    if not host then
        return
    end

    -- Replay every already-spawned object (players, items, monsters, ...) so
    -- the new client can build the whole world. Its own player is spawned
    -- later, when it sends SpawnPlayerRequest.
    for _, record in ipairs(self._spawns) do
        if record.net_id ~= peer:GetID() then
            host:Send(peer, self:buildSpawnMsg(record), 0,
                      TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
        end
    end
end

---@param peer UDPPeer
function _M:onNetDisconnect(peer)
    local ctx = TL_Server.GetContext()

    self:RemovePeer(peer)

    local net_msg = TL_Proto.NetMsg()
    local kill_msg = TL_Proto.Kill()
    kill_msg:set_m_net_id(peer:GetID())

    net_msg:set_m_kill(kill_msg)
    local host = ctx:GetNetHost()
    if host then
        host:Broadcast(net_msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
    end

    TL_Server.GetContext():Log("peer ", peer:GetID(), " disconnected")
end

return _M
