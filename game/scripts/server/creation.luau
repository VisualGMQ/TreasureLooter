local Creation = require("@common/creation")
local PlayerBehavior = require("@client/player_behavior")

local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Creation })

function _M.CreatePrefab(self, scene:Scene, prefab:PrefabHandle, transform:Transform):Entity
    return scene:Instantiate(prefab, transform)
end

function _M.CreateCharacter(self, scene:Scene, spawn_info:ObjectSpawnDefinition, spawn_points:{[string]:any}, object_definitions):Entity
    local ctx = TL_Client.GetContext()

    local spawn_point = spawn_points[spawn_info.m_spawn_point_name]
    if not spawn_point then
        ctx:Log("spawn character failed: no spawn point ", spawn_info.m_spawn_point_name)
        return TL_Common.null_entity
    end

    local transform = TL_Common.Transform()
    transform.m_position = spawn_point.m_position

    local definition = object_definitions:GetCharacter(spawn_info.m_did)
    local prefab = Creation.ConvertCharacterDefinitionToPrefab(definition)
    prefab.m_server_script = spawn_info.m_server_script

    local entity = _M.CreatePrefab(self, scene, prefab, transform)
    local script = ctx:GetScriptManager():Get(entity) :: PlayerBehavior.PlayerBehavior
    if script then
        script:initCharacter(definition)
    end

    ctx:Log("spawn character ", spawn_info.m_did,
            " on SpawnPoint(", spawn_info.m_spawn_point_name, ")",
            ", on Layer(", spawn_info.m_spawn_on_layer, ")")

    return entity
end

return _M
