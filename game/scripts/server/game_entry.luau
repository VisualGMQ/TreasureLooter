local GameEntry = require("@common/game_entry")
local PlayerBehavior = require("@client/player_behavior")
local Creation = require("@common/creation")

local ServerGameEntry = {}
ServerGameEntry.__index = ServerGameEntry
setmetatable(ServerGameEntry, { __index = GameEntry })

type ServerGameEntryData = {}
export type ServerGameEntry = typeof(ServerGameEntry) & ServerGameEntryData & GameEntry.GameEntry

function ServerGameEntry.new(entity: Entity) : ServerGameEntry
    local self = setmetatable(GameEntry.new(entity), ServerGameEntry)
    return self
end

function ServerGameEntry.CreatePrefab(self:ServerGameEntry, scene: Scene, prefab:PrefabHandle, transform:Transform):Entity
    return scene:Instantiate(prefab, transform)
end

function ServerGameEntry.CreateCharacter(self:ServerGameEntry, scene:Scene, spawn_info:ObjectSpawnDefinition):Entity
    local ctx = TL_Client.GetContext()

    local spawn_point = self.m_spawn_points[spawn_info.m_spawn_point_name]
    if not spawn_point then
        ctx:Log("spawn character failed: no spawn point ", spawn_info.m_spawn_point_name)
        return TL_Common.null_entity
    end

    local transform = TL_Common.Transform()
    transform.m_position = spawn_point.m_position

    local definition = self.m_object_definitions:GetCharacter(spawn_info.m_did)
    local prefab = Creation.ConvertCharacterDefinitionToPrefab(definition)
    prefab.m_server_script = spawn_info.m_server_script

    local entity = self:CreatePrefab(scene, prefab, transform)
    local script = ctx:GetScriptManager():Get(entity) :: PlayerBehavior.PlayerBehavior
    if script then
        script:initCharacter(definition)
    end

    ctx:Log("spawn character ", spawn_info.m_did,
            " on SpawnPoint(", spawn_info.m_spawn_point_name, ")",
            ", on Layer(", spawn_info.m_spawn_on_layer, ")")

    return entity
end

return ServerGameEntry