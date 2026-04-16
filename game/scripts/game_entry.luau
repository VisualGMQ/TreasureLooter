local TL = require("@game/tl_types")
local ScriptBehavior = require("@game/script_behavior")
local Creation = require("@game/creation")
local DebugPanel = require("@game/debug_panel")
local CharacterBehavior = require("@game/character_behavior")

local GameEntry = {}
GameEntry.__index = GameEntry
setmetatable(GameEntry, { __index = ScriptBehavior })

type GameEntryData = { }

export type GameEntry = typeof(setmetatable({}::GameEntryData, GameEntry))

function GameEntry.new(entity: TL.Entity) : GameEntry
    local self = ScriptBehavior.new(entity)
    return setmetatable(self, GameEntry)
end

function GameEntry.OnInit(self: GameEntry)
    local ctx: TL.GameContext = TL.GetContext()

    local level_definition = ctx:GetAssetsManager():GetLevelDefinitionManager():Load("assets/gpa/levels/main.level.xml")
    if not level_definition then
        TL.Log("load level definition failed: nil")
        return
    end

    local scene_manager = ctx:GetSceneManager()

    local scene = scene_manager:GetCurrentScene()

    if not scene then
        TL.Log("create scene failed")
        return
    end

    local root_entity = scene:GetRootEntity()
    local root_relationship = ctx:GetRelationshipManager():Get(root_entity)

    local prefab_manager = ctx:GetAssetsManager():GetPrefabManager()

    -- create map layers
    local tilemap = level_definition.m_map_definition.m_map

    local map_layer_entities = {}

    for _, layer: TL.MapLayerDefinition in ipairs(level_definition.m_map_definition.m_layers) do
        local prefab = prefab_manager:Create()

        prefab.m_transform = TL.Transform()
        prefab.m_draw_order = layer.m_draw_order
        prefab.m_tilemap_layer = TL.TilemapLayerDefinition()
        prefab.m_tilemap_layer.m_position = TL.Vec2.ZERO
        prefab.m_tilemap_layer.m_layer_name = layer.m_name
        prefab.m_tilemap_layer.m_tilemap = tilemap

        local entity = scene:Instantiate(prefab)
        map_layer_entities[layer.m_name] = entity
        root_relationship:AddChild(entity)

        prefab_manager:Unload(prefab)
    end

    -- record all spawn points
    local spawn_point_infos = {}

    for i = 0, tilemap:GetLayerCount() - 1 do
        local layer = tilemap:GetLayer(i)
        if (not layer) or layer:GetType() ~= TL.TilemapLayerType.Object then
            continue
        end

        local object_layer = layer:AsObjectLayer()
        if not object_layer then
            continue
        end
        for j = 0, object_layer:GetObjectCount() - 1 do
            local object = object_layer:GetObject(j)
            if object and object:GetType() == TL.TilemapObjectType.Point then
                local point = object:AsPoint()
                if point then
                    spawn_point_infos[object:GetName()] = point
                end
            end
        end
    end

    for name, v in pairs(spawn_point_infos) do
        TL.Log("spaw point: ", name)
    end

    local player_put_on_layer_entity = map_layer_entities[level_definition.m_map_definition.m_player_put_on_layer]
    if not player_put_on_layer_entity then
        TL.Log("no player put on layer")
        return
    end

    ctx:GetRelationshipManager():RegisterEntity(player_put_on_layer_entity)
    local player_put_on_layer_relationship = ctx:GetRelationshipManager():Get(player_put_on_layer_entity)

    -- create player
    local script_mgr = ctx:GetScriptManager()

    local player_spawn_point = spawn_point_infos[level_definition.m_map_definition.m_player_spawn_point]
    if player_spawn_point then
        local definition = ctx:GetAssetsManager():GetCharacterDefinitionManager():Load("assets/gpa/monsters/waggo/waggo.character_def.xml")
        local transform: TL.Transform = TL.Transform()
        transform.m_position = player_spawn_point
        local character = Creation.CreateCharacter(transform, definition)

        if not character then
            TL.Log("failed to create player, entity is null")
            return
        end
        local player_script = script_mgr:Get(character.m_entity)
        if player_script then
            (player_script :: CharacterBehavior.CharacterBehavior):SetTeamID(level_definition.m_map_definition.m_player_team_id)
        end
        TL.Log("create player")

        player_put_on_layer_relationship:AddChild(character:GetEntity())
        TL.Log("put player on layer ", level_definition.m_map_definition.m_player_put_on_layer)
    else
        TL.Log("no player spawn point")
    end

    -- create other components
    for spawn_point_name, spawn_object_info: TL.ObjectSpawnDefinition in pairs(level_definition.m_spawn_objects) do
        local spawn_point = spawn_point_infos[spawn_point_name]

        local asset_path = spawn_object_info.m_asset

        if not spawn_point then
            TL.Log("spawn ", asset_path, " failed: no spawn point ", spawn_point_name)
            continue
        end

        if TL.FilenameIsCharacterDefinition(spawn_object_info.m_asset) then
            local transform = TL.Transform()
            transform.m_position = spawn_point
            TL.Log("spawning ", asset_path)
            local handle = ctx:GetAssetsManager():GetCharacterDefinitionManager():Load(asset_path, false)
            if not handle:IsValid() then
                TL.Log("can't load ", asset_path)
                continue
            end

            local character = Creation.CreateCharacter(transform, handle)
            if character == nil then
                TL.Log("failed to spawn ", asset_path, ", entity is null")
                continue
            end
            local player_script = script_mgr:Get(character.m_entity)
            if player_script then
                (player_script :: CharacterBehavior.CharacterBehavior):SetTeamID(spawn_object_info.m_team_id)
            end
            TL.Log("create object ", asset_path)

            player_put_on_layer_relationship:AddChild(character:GetEntity())
            TL.Log("put object on layer ", level_definition.m_map_definition.m_player_put_on_layer)
        elseif TL.FilenameIsPrefab(asset_path) then
            local prefab = ctx:GetAssetsManager():GetPrefabManager():Load(asset_path)
            if not prefab:IsValid() then
                TL.Log("load prefab ", asset_path, " failed")
                continue
            end

            local entity = scene:Instantiate(prefab)
            TL.Log("create prefab  ", asset_path)
            player_put_on_layer_relationship:AddChild(entity)
            TL.Log("put prefab on layer ", level_definition.m_map_definition.m_player_put_on_layer)
        end
    end
end

function GameEntry.OnRender(self: GameEntry)
    DebugPanel.ShowDebugPanel()
end

return GameEntry
