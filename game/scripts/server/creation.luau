local Creation = require("@common/creation")
local ServerGameObject = require("@server/gameobject")
local ServerGameObjectBehavior = require("@server/gameobject_behavior")
local ServerMoveComponent = require("@server/components/move")

local _M = {}
_M.__index = _M
setmetatable(_M, {__index = Creation})

local k_default_script = TL_Common.Path("scripts/server/gameobject_behavior.luau")

export type ServerCreation = typeof(_M)

function _M.CreatePrefab(self, scene:Scene, prefab:PrefabHandle, transform:Transform):Entity
    return scene:Instantiate(prefab, transform)
end

function _M.CreateCharacter(self, scene:Scene, spawn_info:ObjectSpawnDefinition, position:Vec2, object_definitions, hfsm_definition:ScriptHFSMDefinitionHandle?): (Entity, ServerGameObject.ServerGameObject)
    local ctx = TL_Common.GetContext()

    local transform = TL_Common.Transform()
    transform.m_position = position

    local definition = object_definitions:GetCharacter(spawn_info.m_did)
    local prefab = Creation.ConvertCharacterDefinitionToPrefab(definition)

    if spawn_info.m_server_script:empty() then
        prefab.m_server_script = k_default_script
    else
        prefab.m_server_script = spawn_info.m_server_script
    end

    local entity = _M.CreatePrefab(self, scene, prefab, transform)

    ctx:Log("spawn character ", spawn_info.m_did, " on SpawnPoint(", spawn_info.m_spawn_point_name, ")")

    if definition.m_init_weapon ~= TL_Schema.DID.Invalid then
        ctx:Log("server doesn't create init weapon ", definition.m_init_weapon, " yet")
    end

    local script = ctx:GetScriptManager():Get(entity) :: ServerGameObjectBehavior.ServerGameObjectBehavior
    if script then
        local move_definition = {} :: ServerMoveComponent.ServerMoveComponentDefinition
        move_definition.m_cct = ctx:GetCCTManager():Get(entity)
        move_definition.m_speed = definition.m_move.m_speed

        local go_definition = {} :: ServerGameObject.ServerGameObjectDefinition
        go_definition.m_did = spawn_info.m_did
        go_definition.m_move_component_definition = move_definition
        script:initGameObject(go_definition)
    end

    return entity, script and script.m_gameobject
end

function _M.CreatePlayerHint(self, scene:Scene, player_related_definition:PlayerRelatedDefinition, object_definitions)
end

function _M.CreateItem(self, scene:Scene, spawn_info:ObjectSpawnDefinition, position:Vec2, object_definitions): (Entity, GameObject.GameObject?)
    TL_Common.GetContext():Log("server doesn't support spawning item ", spawn_info.m_did, " yet")
    return TL_Common.null_entity, nil
end

function _M.CreateFX(self, scene:Scene, spawn_info:ObjectSpawnDefinition, position:Vec2, object_definitions): (Entity, GameObject.GameObject?)
    TL_Common.GetContext():Log("server doesn't support spawning fx ", spawn_info.m_did, " yet")
    return TL_Common.null_entity, nil
end

function _M.CreateSkill(self, scene:Scene, spawn_info:ObjectSpawnDefinition, position:Vec2, object_definitions): (Entity, GameObject.GameObject?)
    TL_Common.GetContext():Log("server doesn't support spawning skill ", spawn_info.m_did, " yet")
    return TL_Common.null_entity, nil
end

return _M
