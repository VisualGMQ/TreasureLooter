local GameEntry = require("@common/game_entry")
local ClientCreation = require("@client/creation")
local DebugPanel = require("@client/debug_panel")
local ClientGameObjectBehavior = require("@client/gameobject_behavior")

local ClientGameEntry = {}
ClientGameEntry.__index = ClientGameEntry
setmetatable(ClientGameEntry, { __index = GameEntry })

type ClientGameEntryData = {}
export type ClientGameEntry = typeof(ClientGameEntry) & ClientGameEntryData & GameEntry.GameEntry

ClientGameEntry.k_default_script = TL_Common.Path("scripts/client/gameobject_behavior.luau")

function ClientGameEntry.new(entity: Entity) : ClientGameEntry
    local self = setmetatable(GameEntry.new(entity), ClientGameEntry)
    return self
end

function ClientGameEntry.CreatePrefab(self:ClientGameEntry, scene: Scene, prefab:PrefabHandle, transform:Transform):Entity
    return scene:Instantiate(prefab, transform)
end

function ClientGameEntry.CreateCharacter(self:ClientGameEntry, scene:Scene, spawn_info:ObjectSpawnDefinition):Entity
    local ctx = TL_Client.GetContext()

    local spawn_point = self.m_spawn_points[spawn_info.m_spawn_point_name]
    local transform = TL_Common.Transform()
    if spawn_point then
        transform.m_position = spawn_point.m_position
    else
        ctx:Log("spawn character ", spawn_info.m_did, "'s spawn point is nil, spawn point name = ", spawn_info.m_spawn_point_name)
    end

    local definition = self.m_object_definitions:GetCharacter(spawn_info.m_did)
    local prefab = ClientCreation.ConvertCharacterDefinitionToPrefab(definition)

    if spawn_info.m_client_script:empty() then
        prefab.m_client_script = ClientGameEntry.k_default_script
    else
        prefab.m_client_script = spawn_info.m_client_script
    end

    local entity = self:CreatePrefab(scene, prefab, transform)

    ctx:Log("spawn character ", spawn_info.m_did,
            " on SpawnPoint(", spawn_info.m_spawn_point_name, ")",
            ", on Layer(", spawn_info.m_spawn_on_layer, ")")

    local weapon_entity = TL_Common.null_entity
    if definition.m_init_weapon ~= TL_Schema.DID.Invalid then
        local weapon_spawn_definition = TL_Schema.ObjectSpawnDefinition()
        weapon_spawn_definition.m_did = definition.m_init_weapon
        weapon_entity = self:CreateItem(scene, weapon_spawn_definition)
        local weapon_relationship = ctx:GetRelationshipManager():Get(weapon_entity)
        if weapon_relationship then
            weapon_relationship:RemoveFromParent()
            local parent_relationship = ctx:GetRelationshipManager():Get(entity)
            parent_relationship:AddChild(weapon_entity)
        else
            ctx:Log("weapon don't has relationship component")
        end
    end

    local script = ctx:GetScriptManager():Get(entity) :: ClientGameObjectBehavior.ClientGameObjectBehavior
    if script then
        local go_definition = ClientCreation.ConvertCharacterDefinitionToGODefinition(entity, definition, weapon_entity)
        script:initGameObject(go_definition)
    end

    return entity
end

function ClientGameEntry.CreateItem(self:ClientGameEntry, scene:Scene, spawn_info:ObjectSpawnDefinition): Entity
    local ctx = TL_Client.GetContext()

    local spawn_point = self.m_spawn_points[spawn_info.m_spawn_point_name]
    local transform = TL_Common.Transform()
    if spawn_point then
        transform.m_position = spawn_point.m_position
    else
        ctx:Log("spawn item ", spawn_info.m_did, "'s spawn point is nil: spawn point name = ", spawn_info.m_spawn_point_name)
    end

    local definition = self.m_object_definitions:GetItem(spawn_info.m_did)
    local prefab = ClientCreation.ConvertItemDefinitionToPrefab(definition)

    if spawn_info.m_client_script:empty() then
        prefab.m_client_script = ClientGameEntry.k_default_script
    else
        prefab.m_client_script = spawn_info.m_client_script
    end

    local entity = self:CreatePrefab(scene, prefab, transform)
    local script = ctx:GetScriptManager():Get(entity) :: ClientGameObjectBehavior.ClientGameObjectBehavior
    if script then
        local go_definition = ClientCreation.ConvertItemDefinitionToGODefinition(entity, definition)
        script:initGameObject(go_definition)
    end

    ctx:Log("spawn item ", spawn_info.m_did,
            " on SpawnPoint(", spawn_info.m_spawn_point_name, ")",
            ", on Layer(", spawn_info.m_spawn_on_layer, ")")

    return entity
end

function ClientGameEntry.OnRender(self:ClientGameEntry)
    DebugPanel.ShowDebugPanel()
end

return ClientGameEntry