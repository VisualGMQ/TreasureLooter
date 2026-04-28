local TL_Common = require("@type_hints/tl_common_types")
local TL_Client = require("@type_hints/tl_client_types")
local TL_Schema = require("@type_hints/tl_schema_types")


local ScriptBehavior = require("@client/script_behavior")
local Creation = require("@client/creation")
local DebugPanel = require("@client/debug_panel")
local PlayerBehavior = require("@client/player_behavior")
local LuaContext = require("@client/context")

local GameEntry = {}
GameEntry.__index = GameEntry
setmetatable(GameEntry, { __index = ScriptBehavior })

type GameEntryData = { }

export type GameEntry = typeof(setmetatable({}::GameEntryData, GameEntry))

function GameEntry.new(entity: TL_Common.Entity) : GameEntry
    local self = ScriptBehavior.new(entity)
    return setmetatable(self, GameEntry)
end

function GameEntry.OnInit(self: GameEntry)
    local ctx: TL_Client.ClientContext = TL_Client.GetClientContext()

    local level_definition = ctx:GetAssetsManager():GetLevelDefinitionManager():Load("assets/gpa/levels/main.level.xml")
    if not level_definition then
        TL_Common.Log("load level definition failed: nil")
        return
    end

    local scene_manager = ctx:GetSceneManager()

    local scene = scene_manager:GetCurrentScene()

    if not scene then
        TL_Common.Log("create scene failed")
        return
    end

    local root_entity = scene:GetRootEntity()
    local root_relationship = ctx:GetRelationshipManager():Get(root_entity)

    local prefab_manager = ctx:GetAssetsManager():GetPrefabManager()

    -- create map layers
    local tilemap = level_definition.m_map_definition.m_map

    local map_layer_entities = {}

    for _, layer: TL_Schema.MapLayerDefinition in ipairs(level_definition.m_map_definition.m_layers) do
        local prefab = prefab_manager:Create()

        prefab.m_transform = TL_Common.Transform()
        prefab.m_draw_order = layer.m_draw_order
        prefab.m_tilemap_layer = TL_Schema.TilemapLayerDefinition()
        prefab.m_tilemap_layer.m_position = TL_Common.Vec2.ZERO
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
                    spawn_point_infos[object:GetName()] = point
                end
            end
        end
    end

    for name, v in pairs(spawn_point_infos) do
        TL_Common.Log("spaw point: ", name)
    end

    local player_put_on_layer_entity = map_layer_entities[level_definition.m_map_definition.m_player_put_on_layer]
    if not player_put_on_layer_entity then
        TL_Common.Log("no player put on layer")
        return
    end

    local player_put_on_layer_relationship = ctx:GetRelationshipManager():Get(player_put_on_layer_entity)

    -- record global state
    LuaContext.m_player_layer_entity = player_put_on_layer_entity

    -- create player
    local script_mgr = ctx:GetScriptManager()

    local player_spawn_point = spawn_point_infos[level_definition.m_map_definition.m_player_spawn_point]
    if player_spawn_point then
        local definition = level_definition.m_map_definition.m_player_character
        local transform: TL_Common.Transform = TL_Common.Transform()
        transform.m_position = player_spawn_point

        local entity, character = Creation.CreateCharacter(transform, definition)

        if not character then
            TL_Common.Log("failed to create player, entity is null")
            return
        end
        character:SetTeamID(level_definition.m_map_definition.m_player_team_id)
        local player_script = script_mgr:Get(entity)
        if player_script then
            local behavior = player_script :: PlayerBehavior.PlayerBehavior
            behavior:_init(character, level_definition.m_map_definition.m_player_related_definition)
        end
        TL_Common.Log("create player")

        player_put_on_layer_relationship:AddChild(entity)
        TL_Common.Log("put player on layer ", level_definition.m_map_definition.m_player_put_on_layer)
    else
        TL_Common.Log("no player spawn point")
    end

    -- create other components
    for spawn_point_name, spawn_object_info: TL_Schema.ObjectSpawnDefinition in pairs(level_definition.m_spawn_objects) do
        local spawn_point = spawn_point_infos[spawn_point_name]

        local asset_path = spawn_object_info.m_asset

        TL_Common.Log("spawning ", asset_path, " at spawn point ", spawn_point_name)

        if not spawn_point then
            TL_Common.Log("spawn failed: no spaw point")
            continue
        end

        if TL_Schema.FilenameIsCharacterDefinition(spawn_object_info.m_asset) then
            local transform = TL_Common.Transform()
            transform.m_position = spawn_point
            local handle = ctx:GetAssetsManager():GetCharacterDefinitionManager():Load(asset_path, false)
            if not handle:IsValid() then
                TL_Common.Log("spawn failed: can't load asset")
                continue
            end

            local entity, character = Creation.CreateCharacter(transform, handle)
            if character == nil then
                TL_Common.Log("spawn failed: create character failed")
                continue
            end
            character:SetTeamID(spawn_object_info.m_team_id)
            local script = script_mgr:Get(entity)
            if script then
                script:_init(character)
            end

            player_put_on_layer_relationship:AddChild(entity)
            TL_Common.Log("put object on layer ", level_definition.m_map_definition.m_player_put_on_layer)
        elseif TL_Schema.FilenameIsPrefab(asset_path) then
            local prefab = ctx:GetAssetsManager():GetPrefabManager():Load(asset_path)
            if not prefab:IsValid() then
                TL_Common.Log("spawn failed: can't load prefab")
                continue
            end

            local entity = scene:Instantiate(prefab)
            player_put_on_layer_relationship:AddChild(entity)
            TL_Common.Log("put prefab on layer ", level_definition.m_map_definition.m_player_put_on_layer)
        elseif TL_Schema.FilenameIsItemDefinition(asset_path) then
            local transform = TL_Common.Transform()
            transform.m_position = spawn_point
            local handle = ctx:GetAssetsManager():GetItemDefinitionManager():Load(asset_path, false)
            if not handle:IsValid() then
                TL_Common.Log("spawn failed: can't load asset")
                continue
            end

            local entity, item = Creation.CreateItem(transform, handle)
            if entity == TL_Common.null_entity then
                TL_Common.Log("spawn failed: create item failed")
                continue
            end

            local script = script_mgr:Get(entity)

            if script then
                script:_init(item)
            end

            player_put_on_layer_relationship:AddChild(entity)
            TL_Common.Log("put object on layer ", level_definition.m_map_definition.m_player_put_on_layer)
        else
            TL_Common.Log("spawn failed: no support file type")
        end
    end
end

function GameEntry.OnRender(self: GameEntry)
    DebugPanel.ShowDebugPanel()
end

return GameEntry


