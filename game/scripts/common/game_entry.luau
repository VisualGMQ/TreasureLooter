local ScriptBehavior = require("@common/script_behavior")
local ObjectDefinitionTable = require("@common/object_def_table")
local DID = require("@common/did")
local World = require("@common/world")
local Creation = require("@common/creation")

local GameEntry = {}
GameEntry.__index = GameEntry
setmetatable(GameEntry, { __index = ScriptBehavior })

type SpawnPoint = {
    m_name:string,
    m_position:Vec2,
}

type GameEntryData = {
    m_level_definition:LevelDefinitionHandle,
    m_map_layers:{[string]:Entity},
    m_spawn_points:{[string]:SpawnPoint},
    m_player_on_layer:Entity,

    m_object_definitions: ObjectDefinitionTable.ObjectDefinitionTable,
    m_creation_strategy: typeof(Creation),
}

export type GameEntry = typeof(GameEntry) & GameEntryData & ScriptBehavior.ScriptBehavior

function GameEntry.new(entity: Entity, creation_strategy: typeof(Creation)) : GameEntry
    local self = ScriptBehavior.new(entity) :: GameEntryData
    self.m_creation_strategy = creation_strategy
    return setmetatable(self, GameEntry)
end

-- function GameEntry.initNet()
--     local Common = require("@common/net")
--     local ctx: ClientContext = TL_Client.GetContext()
--     local net_address = TL_Common.NetAddress(Common.ip, Common.port)
--     ctx:ConnectToServer(net_address)
--
--     local net_msg: TL_Proto.NetMsg = TL_Proto.NetMsg()
--     local echo_msg: TL_Proto.Echo = TL_Proto.Echo()
--     echo_msg:set_m_msg("Hello, I'm Client")
--     net_msg:set_m_echo(echo_msg)
--     local host = ctx:GetNetHost()
--     host:Send(ctx:GetNetPeer(), net_msg, 0, TL_Common.UDPPacketFlags(TL_Common.UDPPacketFlag.Reliable))
-- end

function GameEntry.OnInit(self: GameEntry)
    local ctx = TL_Common.GetContext()
    local object_definition_table_handle = ctx:GetAssetsManager():GetObjectDefinitionTableManager():Load("assets/gpa/object_definition_table.object_definition_table.xml")
    self.m_object_definitions = ObjectDefinitionTable.new(object_definition_table_handle)
    World.GetInst().m_object_definitions = self.m_object_definitions
    self:ChangeLevel(TL_Common.Path("assets/gpa/levels/main.level.xml"))
end

function GameEntry.ChangeLevel(self: GameEntry, level: Path)
    local ctx = TL_Client.GetContext()

    self.m_level_definition = ctx:GetAssetsManager():GetLevelDefinitionManager():Load(level)
    local new_scene_definition = ctx:GetAssetsManager():GetSceneDefinitionManager():Create()
    local new_scene = ctx:GetSceneManager():Create(new_scene_definition)

    local scene_mgr = ctx:GetSceneManager()
    scene_mgr:Switch(new_scene)
    ctx:GetAssetsManager():GetSceneDefinitionManager():Unload(new_scene_definition)

    local scene = scene_mgr:GetCurrentScene()
    if not scene then
        ctx:Log("create scene failed")
        return
    end

    ctx:Log("change level to ", level)
    self:InitSceneFromLevelDefinition(scene, self.m_level_definition)
end

function GameEntry.gatherSpawnPoints(level_definition: LevelDefinitionHandle, map_layer_entities:{[string]: Entity}): {[string]: SpawnPoint}
    local spawn_point_infos = {}

    local tilemap = level_definition.m_map_definition.m_map

    for i = 0, tilemap:GetLayerCount() - 1 do
        local layer = tilemap:GetLayer(i)
        if (not layer) or layer:GetType() ~= TL_Common.TilemapLayerType.Object then
            continue
        end

        local object_layer = layer:AsObjectLayer()
        if not object_layer then
            continue
        end
        for j = 0, object_layer:GetObjectCount() - 1 do
            local object = object_layer:GetObject(j)
            if object and object:GetType() == TL_Common.TilemapObjectType.Point then
                local point = object:AsPoint()
                if point then
                    spawn_point_infos[object:GetName()] = {
                        m_name = object:GetName(),
                        m_position = point,
                    }
                end
            end
        end
    end

    -- for debug
    for name, v in pairs(spawn_point_infos) do
        TL_Common.GetContext():Log("spawn point: ", name)
    end

    return spawn_point_infos
end

function GameEntry.createMapLayers(scene: Scene, root_relationship: Relationship, map_definition:MapDefinition, land:string):{[string]:Entity}
    local ctx = TL_Common.GetContext()
    local prefab_mgr = ctx:GetAssetsManager():GetPrefabManager()

    local map_layer_entities = {}
    for _, layer: MapLayerDefinition in ipairs(map_definition.m_layers) do
        local prefab = prefab_mgr:Create()

        prefab.m_transform = TL_Common.Transform()
        prefab.m_draw_order = layer.m_draw_order
        prefab.m_tilemap_layer = TL_Schema.TilemapLayerDefinition()
        prefab.m_tilemap_layer.m_position = TL_Common.Vec2.ZERO
        prefab.m_tilemap_layer.m_layer_name = layer.m_name
        prefab.m_tilemap_layer.m_tilemap = map_definition.m_map

        local entity = scene:Instantiate(prefab, nil)
        map_layer_entities[layer.m_name] = entity
        root_relationship:AddChild(entity)

        if layer.m_name == land then
            World.GetInst().m_land_entity = entity
        end

        prefab_mgr:Unload(prefab)
    end

    return map_layer_entities
end

function GameEntry.InitSceneFromLevelDefinition(self: GameEntry, scene: Scene, level_definition: LevelDefinitionHandle)
    local ctx = TL_Client.GetContext()
    local root_entity = scene:GetRootEntity()
    local root_relationship = ctx:GetRelationshipManager():Get(root_entity)

    self.m_spawn_points = self.gatherSpawnPoints(level_definition, self.m_map_layers)
    self.m_map_layers = self.createMapLayers(scene, root_relationship, level_definition.m_map_definition, level_definition.m_land)

    self.m_creation_strategy:CreatePlayerHint(scene, level_definition.m_player_related_definition, self.m_object_definitions)

    -- create objects
    for _, spawn_info in ipairs(level_definition.m_spawn_objects) do
        local did = spawn_info.m_did
        local spawn_point = self.m_spawn_points[spawn_info.m_spawn_point_name]

        if not spawn_point then
            ctx:Log("spawn failed: can't find spawn point ", spawn_point, " for object ", spawn_info.m_did)
            continue
        end

        local transform = TL_Common.Transform()
        transform.m_position = spawn_point.m_position

        local target_entity = self.m_map_layers[spawn_info.m_spawn_on_layer]
        ctx:Log("spawn point name ", spawn_point.m_name)
        local relationship = ctx:GetRelationshipManager():Get(target_entity)

        local definition = self.m_object_definitions:Get(spawn_info.m_did)

        if DID.IsCharacterDID(spawn_info.m_did) then
            local entity = self.m_creation_strategy:CreateCharacter(scene, spawn_info, self.m_spawn_points, self.m_object_definitions)
            relationship:AddChild(entity)
        elseif DID.IsItemDID(did) then
            local entity = self.m_creation_strategy:CreateItem(scene, spawn_info, self.m_spawn_points, self.m_object_definitions)
            relationship:AddChild(entity)
        elseif TL_Schema.FilenameIsPrefab(definition:GetFilename()) then
            local prefab = ctx:GetAssetsManager():GetPrefabManager():Load(definition:GetFilename())
            if not prefab:IsValid() then
                ctx:Log("spawn failed: can't load prefab")
                continue
            end

            local entity = self.m_creation_strategy:CreatePrefab(scene, prefab, transform)
            relationship:AddChild(entity)
        else
            ctx:Log("spawn failed: no support file type")
        end
    end
end

function GameEntry.OnRender(self: GameEntry)
end

return GameEntry


