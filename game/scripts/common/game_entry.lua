local ScriptBehavior = require("common.script_behavior")
local ObjectDefinitionTable = require("common.object_def_table")
local BuffApplierTable = require("common.buff.buff_applier_table")
local DID = require("common.did")
local World = require("common.world")

---@class SpawnPoint
---@field m_name string
---@field m_position Vec2

---@class GameEntry : ScriptBehavior
---@field m_map_layers table<string, LogicEntity>
---@field m_creation_strategy Creation
local GameEntry = {}
GameEntry.__index = GameEntry
setmetatable(GameEntry, { __index = ScriptBehavior })

---@param entity LogicEntity
---@param creation_strategy Creation
---@return GameEntry
function GameEntry.new(entity, creation_strategy)
    local self = ScriptBehavior.new(entity)
    ---@cast self GameEntry
    self = setmetatable(self, GameEntry)
    self.m_creation_strategy = creation_strategy
    return self
end

function GameEntry:OnInit()
    local ctx = TL_Common.GetContext()
    local object_definition_table_handle = ctx:GetAssetsManager():GetObjectDefinitionTableManager():Load(
    "assets/gpa/object_definition_table.object_definition_table.xml")
    self.m_object_definitions = ObjectDefinitionTable.new(object_definition_table_handle)
    local world = World.GetInst()
    world.m_object_definitions = self.m_object_definitions
    world.m_buff_appliers = BuffApplierTable.new()
    self:LoadLevel(TL_Common.Path("assets/gpa/levels/main.level.xml"))
end

---@param level Path
function GameEntry:LoadLevel(level)
    local ctx = TL_Common.GetContext()
    local world = World.GetInst()

    local level_definition = ctx:GetAssetsManager():GetLevelDefinitionManager():Load(level)
    world.m_level_definition = level_definition
    ctx:Log("load level", level)

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        ctx:Log("load level failed: no current scene")
        return
    end
    self:InitSceneFromLevelDefinition(scene, level_definition)
end

---@param level_definition LevelDefinitionHandle
---@param map_layer_entities table<string, LogicEntity>
---@return table<string, SpawnPoint>
function GameEntry.gatherSpawnPoints(level_definition, map_layer_entities)
    local spawn_point_infos = {}

    local tilemap = level_definition.m_map_definition.m_map

    for i = 0, tilemap:GetLayerCount() - 1 do
        local layer = tilemap:GetLayer(i)
        if (not layer) or layer:GetType() ~= TL_Common.TilemapLayerType.Object then
            goto continue
        end

        local object_layer = layer:AsObjectLayer()
        if not object_layer then
            goto continue
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
        ::continue::
    end

    -- for debug
    for name, v in pairs(spawn_point_infos) do
        TL_Common.GetContext():Log("spawn point: ", name)
    end

    return spawn_point_infos
end

---@param scene Scene
---@param root_relationship Relationship
---@param map_definition MapDefinition
---@param land string
---@return table<string, LogicEntity>
function GameEntry.createMapLayers(scene, root_relationship, map_definition, land)
    local ctx = TL_Common.GetContext()
    local prefab_mgr = ctx:GetAssetsManager():GetPrefabManager()

    local map_layer_entities = {}
    for _, layer in ipairs(map_definition.m_layers) do
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

---@param scene Scene
---@param level_definition LevelDefinitionHandle
function GameEntry:InitSceneFromLevelDefinition(scene, level_definition)
    local ctx = TL_Common.GetContext()
    local world = World.GetInst()
    local root_entity = scene:GetRootEntity()
    local root_relationship = ctx:GetRelationshipManager():Get(root_entity)
    if not root_relationship then
        ctx:Log("init scene failed: root entity has no relationship component")
        return
    end

    local spawn_points = self.gatherSpawnPoints(level_definition, self.m_map_layers)
    world.m_spawn_points = spawn_points
    self.m_map_layers = self.createMapLayers(scene, root_relationship, level_definition.m_map_definition, level_definition.m_land)

    if level_definition.m_map_definition.m_detour:IsValid() then
        ctx:GetTilemapDetourManager():SetCurDetourData(level_definition.m_map_definition.m_detour)
    end

    self.m_creation_strategy:CreatePlayerHint(scene, level_definition.m_player_related_definition, self.m_object_definitions)

    local player_hfsm_handle = nil
    local hfsm_path = level_definition.m_player_related_definition.m_hfsm
    if not hfsm_path:empty() then
        player_hfsm_handle = ctx:GetAssetsManager():GetScriptHFSMDefinitionManager():Load(hfsm_path)
    end

    -- create objects
    for _, spawn_info in ipairs(level_definition.m_spawn_objects) do
        local did = spawn_info.m_did
        local spawn_point = spawn_points[spawn_info.m_spawn_point_name]

        if not spawn_point then
            ctx:Log("spawn failed: can't find spawn point ", spawn_point, " for object ", spawn_info.m_did)
            goto continue
        end

        local transform = TL_Common.Transform()
        transform.m_position = spawn_point.m_position

        local target_entity = self.m_map_layers[spawn_info.m_spawn_on_layer]
        ctx:Log("spawn point name ", spawn_point.m_name)
        local relationship = ctx:GetRelationshipManager():Get(target_entity)
        if not relationship then
            ctx:Log("spawn failed: target layer has no relationship component")
            goto continue
        end

        local definition = self.m_object_definitions:Get(spawn_info.m_did)

        if DID.IsCharacterDID(spawn_info.m_did) then
            local hfsm_definition = nil
            if spawn_info.m_team_id == TL_Schema.TeamID.team1 then
                hfsm_definition = player_hfsm_handle
            end
            local entity = self.m_creation_strategy:CreateCharacter(scene, spawn_info, spawn_point.m_position, 0, self.m_object_definitions, hfsm_definition)
            relationship:AddChild(entity)
        elseif DID.IsItemDID(did) then
            local entity = self.m_creation_strategy:CreateItem(scene, spawn_info, spawn_point.m_position, self.m_object_definitions)
            relationship:AddChild(entity)
        elseif DID.IsFXDID(did) then
            local entity = self.m_creation_strategy:CreateFX(scene, spawn_info, spawn_point.m_position, self.m_object_definitions)
            relationship:AddChild(entity)
        elseif DID.IsSkillDID(did) then
            local entity = self.m_creation_strategy:CreateSkill(scene, spawn_info, spawn_point.m_position, self.m_object_definitions)
            relationship:AddChild(entity)
        else
            local filename = definition and definition:GetFilename()
            if filename and TL_Schema.FilenameIsPrefab(filename) then
                local prefab = ctx:GetAssetsManager():GetPrefabManager():Load(filename)
                if not prefab:IsValid() then
                    ctx:Log("spawn failed: can't load prefab")
                    goto continue
                end

                local entity = self.m_creation_strategy:CreatePrefab(scene, prefab, transform)
                relationship:AddChild(entity)
            else
                ctx:Log("spawn failed: no support file type")
            end
        end
        ::continue::
    end
end

function GameEntry:OnRender()
end

return GameEntry
