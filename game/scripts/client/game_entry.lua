local GameEntry = require("common.game_entry")
local ClientCreation = require("client.creation")
local DebugPanel = require("client.debug_panel")
local DebugCommands = require("client.debug_commands")
local ClientWorld = require("client.world")

---@class ClientGameEntry : GameEntry
local ClientGameEntry = {}
ClientGameEntry.__index = ClientGameEntry
setmetatable(ClientGameEntry, { __index = GameEntry })

---@param entity Entity
---@return ClientGameEntry
function ClientGameEntry.new(entity)
    local self = setmetatable(GameEntry.new(entity, ClientCreation), ClientGameEntry)
    ---@cast self ClientGameEntry
    return self
end

function ClientGameEntry:OnInit()
    ClientWorld.SetInst(ClientWorld.new())
    GameEntry.OnInit(self)
    DebugCommands.RegisterAllDebugCommand()
    self:initNet()
end

function ClientGameEntry:initNet()
    local ctx = TL_Client.GetContext()
    local Common = require("common.net")

    ctx:GetEventSystem():AddNetMsg_ConnectEvent(function(id, peer, net_msg)
        local spawn = TL_Proto.SpawnPlayer()
        spawn:set_m_entity(0)
        spawn:set_m_did(TL_Schema.DID.CharacterDID1)

        local msg = TL_Proto.NetMsg()
        msg:set_m_spawn_player(spawn)

        local host = ctx:GetNetHost()
        if host then
            host:Send(ctx:GetNetPeer(), msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
            ctx:Log("client sent SpawnPlayer")
        end
    end)

    ctx:ConnectToServer(TL_Common.NetAddress(Common.ip, Common.port))
    ctx:Log("client connecting to ", Common.ip, ":", Common.port)
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
