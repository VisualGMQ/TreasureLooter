local GameEntry = require("@common/game_entry")
local ClientCreation = require("@client/creation")
local DebugPanel = require("@client/debug_panel")
local ClientGameObjectBehavior = require("@client/gameobject_behavior")
local ClientWorld = require("@client/world")

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

function ClientGameEntry.OnInit(self:ClientGameEntry)
    ClientWorld.SetInst(ClientWorld.new())
    GameEntry.OnInit(self)
end

function ClientGameEntry.CreatePlayerHint(self:ClientGameEntry, scene:Scene, player_related_definition:PlayerRelatedDefinition)
    local did = player_related_definition.m_hint_fx_did
    if did == TL_Schema.DID.Invalid then
        return
    end
    local fx_definition = self.m_object_definitions:GetFX(did)
    TL_Client.GetContext():Log("creating player hint")
    ClientCreation.CreatePlayerHintFX(scene, did, fx_definition, ClientGameEntry.k_default_script)
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

    local controller_entity = TL_Common.null_entity
    if definition.m_init_weapon ~= TL_Schema.DID.Invalid then
        -- 1. create the weapon item (single entity: sprite + hit-area trigger + item component)
        local weapon_spawn_definition = TL_Schema.ObjectSpawnDefinition()
        weapon_spawn_definition.m_did = definition.m_init_weapon
        local weapon_item_entity = self:CreateItem(scene, weapon_spawn_definition)
        local weapon_def = self.m_object_definitions:GetItem(definition.m_init_weapon)

        -- 2. create the weapon controller under the character
        controller_entity = self:createWeaponController(scene, entity)

        -- 3. put the weapon item under the controller
        local weapon_item_relationship = ctx:GetRelationshipManager():Get(weapon_item_entity)
        if weapon_item_relationship then
            weapon_item_relationship:RemoveFromParent()
            local controller_relationship = ctx:GetRelationshipManager():Get(controller_entity)
            controller_relationship:AddChild(weapon_item_entity)
        else
            ctx:Log("weapon item don't has relationship component")
        end

        -- 4. the controller hosts the attack & weapon components, driven by the player
        local controller_script = ctx:GetScriptManager():Get(controller_entity)
        if controller_script then
            local controller_go_definition = ClientCreation.ConvertWeaponControllerToGODefinition(controller_entity, weapon_item_entity, definition.m_init_weapon, weapon_def)
            controller_script:initGameObject(controller_go_definition)
        end
    end

    local script = ctx:GetScriptManager():Get(entity) :: ClientGameObjectBehavior.ClientGameObjectBehavior
    if script then
        local go_definition = ClientCreation.ConvertCharacterDefinitionToGODefinition(entity, spawn_info.m_did, definition, controller_entity)
        script:initGameObject(go_definition)
    end

    return entity
end

function ClientGameEntry.createWeaponController(self:ClientGameEntry, scene:Scene, character_entity:Entity):Entity
    local ctx = TL_Client.GetContext()

    local prefab = ClientCreation.ConvertWeaponControllerToPrefab()
    prefab.m_client_script = ClientGameEntry.k_default_script
    local controller_entity = scene:Instantiate(prefab, nil)
    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    local controller_relationship = ctx:GetRelationshipManager():Get(controller_entity)
    if controller_relationship then
        controller_relationship:RemoveFromParent()
        local character_relationship = ctx:GetRelationshipManager():Get(character_entity)
        character_relationship:AddChild(controller_entity)
    end

    return controller_entity
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
        local go_definition = ClientCreation.ConvertItemDefinitionToGODefinition(entity, spawn_info.m_did, definition)
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