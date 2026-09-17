local GameEntry = require("common.game_entry")
local ClientCreation = require("client.creation")
local DebugPanel = require("client.debug_panel")
local DebugCommands = require("client.debug_commands")
local ClientWorld = require("client.world")

---@class ClientGameEntry : GameEntry
local ClientGameEntry = {}
ClientGameEntry.__index = ClientGameEntry
setmetatable(ClientGameEntry, { __index = GameEntry })

local k_player_client_script = TL_Common.Path("scripts/client/behaviors/net_player_behavior.lua")

---@param entity LogicEntity
---@return ClientGameEntry
function ClientGameEntry.new(entity)
    local self = setmetatable(GameEntry.new(entity, ClientCreation), ClientGameEntry)
    ---@cast self ClientGameEntry
    return self
end

function ClientGameEntry:OnInit()
    local world = ClientWorld.new()
    ClientWorld.SetInst(world)
    GameEntry.OnInit(self)
    DebugCommands.RegisterAllDebugCommand()
    self:initNet()
end

function ClientGameEntry:initNet()
    local ctx = TL_Client.GetContext()
    local Common = require("common.net")

    ctx:GetEventSystem():AddNetMsg_ConnectEvent(function(id, peer, net_msg)
        local request = TL_Proto.SpawnPlayerRequest()
        request:set_m_entity(0)
        request:set_m_did(TL_Schema.DID.CharacterDID1)

        local msg = TL_Proto.NetMsg()
        msg:set_m_spawn_player_request(request)

        local host = ctx:GetNetHost()
        if host then
            host:Send(ctx:GetNetPeer(), msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
            ctx:Log("client sent SpawnPlayerRequest")
        end
    end)

    ctx:GetEventSystem():AddNetMsg_SpawnPlayerReplyEvent(function(id, peer, reply)
        self:onSpawnPlayerReply(reply)
    end)

    ctx:ConnectToServer(TL_Common.NetAddress(Common.ip, Common.port))
    ctx:Log("client connecting to ", Common.ip, ":", Common.port)
end

--- The server is authoritative about where players spawn; only create the local
--- character once the reply arrives.
---@param reply ProtoSpawnPlayerReply
function ClientGameEntry:onSpawnPlayerReply(reply)
    local ctx = TL_Client.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return
    end

    local did = reply:m_did()
    local net_id = ctx:GetNetPeer():GetID()
    local net_position = reply:m_position()
    local position = TL_Common.Vec2(net_position:m_x(), net_position:m_y())

    local spawn_info = TL_Schema.ObjectSpawnDefinition()
    spawn_info.m_did = did
    spawn_info.m_client_script = k_player_client_script

    local hfsm_definition = nil
    local hfsm_path = self.m_level_definition and self.m_level_definition.m_player_related_definition.m_hfsm
    if hfsm_path and not hfsm_path:empty() then
        hfsm_definition = ctx:GetAssetsManager():GetScriptHFSMDefinitionManager():Load(hfsm_path)
    end

    local entity = ClientCreation.CreateCharacter(ClientCreation, scene, spawn_info,
                            position, net_id, self.m_object_definitions, hfsm_definition)
    if entity == TL_Common.null_entity then
        return
    end

    local root_relationship = ctx:GetRelationshipManager():Get(scene:GetRootEntity())
    if root_relationship then
        root_relationship:AddChild(entity)
    end
    ctx:Log("client spawned local player, net_id ", net_id, " did ", did)
end

function ClientGameEntry:OnRender()
    DebugPanel.ShowDebugPanel()
end

---@param map_definition MapDefinition
function ClientGameEntry:setCameraBoundary(map_definition)
    local ctx = TL_Client.GetContext()
    local tilemap = map_definition.m_map
    if not tilemap then
        return
    end

    local map_size_in_tiles = nil
    for i = 0, tilemap:GetLayerCount() - 1 do
        local layer = tilemap:GetLayer(i)
        local tiled_layer = layer and layer:AsTiledLayer()
        if tiled_layer then
            map_size_in_tiles = tiled_layer:GetSize()
            break
        end
    end

    if not map_size_in_tiles then
        return
    end

    local world_size = tilemap:GetTileSize() * map_size_in_tiles
    local boundary = TL_Schema.Rect()
    boundary.m_center = world_size * 0.5
    boundary.m_half_size = world_size * 0.5
    ctx:Log("set camera boundary, center: ", boundary.m_center, ", half_size: ", boundary.m_half_size)
    ctx:GetCamera():SetBoundary(boundary)
end

---@param scene Scene
---@param level_definition LevelDefinitionHandle
function ClientGameEntry:InitSceneFromLevelDefinition(scene, level_definition)
    GameEntry.InitSceneFromLevelDefinition(self, scene, level_definition)
    self:setCameraBoundary(level_definition.m_map_definition)
end

return ClientGameEntry
