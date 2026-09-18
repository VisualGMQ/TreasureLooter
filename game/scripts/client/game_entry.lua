local GameEntry = require("common.game_entry")
local ClientCreation = require("client.creation")
local DebugPanel = require("client.debug_panel")
local DebugCommands = require("client.debug_commands")
local ClientWorld = require("client.world")

---@class ClientGameEntry : GameEntry
local ClientGameEntry = {}
ClientGameEntry.__index = ClientGameEntry
setmetatable(ClientGameEntry, { __index = GameEntry })

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
    world:RegisterNetEventHandler()

    local ctx = TL_Client.GetContext()
    self:initNet()
end

function ClientGameEntry:initNet()
    local ctx = TL_Client.GetContext()
    local Common = require("common.net")
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
