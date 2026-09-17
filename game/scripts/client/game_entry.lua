local GameEntry = require("common.game_entry")
local ClientCreation = require("client.creation")
local DebugPanel = require("client.debug_panel")
local DebugCommands = require("client.debug_commands")
local ClientWorld = require("client.world")

---@class ClientGameEntry : GameEntry
---@field private _virtual_controller_scene SceneHandle?
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
    local world = ClientWorld.FindInst()
    if not world then
        world = ClientWorld.new()
        ClientWorld.SetInst(world)
        world:RegisterNetEventHandler()
    end
    ---@cast world ClientWorld

    GameEntry.OnInit(self)

    -- The lobby switches to this scene when the server creates the characters,
    -- but their SpawnPlayerReply can already have arrived while the client was
    -- still waiting: create them now that the level is loaded, then tell the
    -- server this client is ready for the countdown.
    world:flushPendingSpawns()
    world:sendPrepareGameFinish()

    -- Android only (a no-op on the other platforms): the virtual joystick and
    -- attack button are per scene, so they are created for the scene that is
    -- actually played instead of the entry/title scene. The handle is kept:
    -- `OnQuit` runs after the switch, when the current scene is already the new
    -- one.
    local ctx = TL_Client.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if scene then
        ---@cast scene SceneHandle
        self._virtual_controller_scene = scene
        ctx:GetPlayerController():RegisterVirtualController(scene)
    end

    DebugCommands.RegisterAllDebugCommand()
end

function ClientGameEntry:OnRender()
    DebugPanel.ShowDebugPanel()
end

--- The game scene is going away: stop its per-round timers and forget the
--- entities that belong to it, while keeping the world (and its net listeners)
--- alive for the next round.
function ClientGameEntry:OnQuit()
    if self._virtual_controller_scene then
        TL_Client.GetContext():GetPlayerController():DestroyVirtualController(
            self._virtual_controller_scene)
        self._virtual_controller_scene = nil
    end

    local world = ClientWorld.FindInst()
    if world then
        ---@cast world ClientWorld
        world:onGameSceneQuit()
    end
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
