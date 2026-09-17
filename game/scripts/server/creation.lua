local Creation = require("common.creation")

---@class ServerCreation : Creation
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Creation })

local k_default_script = TL_Common.Path("scripts/server/gameobject_behavior.lua")

---@param self ServerCreation
---@param scene Scene
---@param prefab PrefabHandle
---@param transform Transform
---@return LogicEntity
function _M:CreatePrefab(scene, prefab, transform)
    return scene:Instantiate(prefab, transform)
end

---@param self ServerCreation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param net_id number
---@param object_definitions ObjectDefinitionTable
---@param hfsm_definition ScriptHFSMDefinitionHandle|nil
---@return LogicEntity, ServerGameObject
function _M:CreateCharacter(scene, spawn_info, position, net_id, object_definitions, hfsm_definition)
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

    local script = ctx:GetScriptManager():Get(entity)
    if script then
        local move_definition = {}
        move_definition.m_cct = ctx:GetCCTManager():Get(entity)
        move_definition.m_speed = definition.m_move.m_speed

        ---@type ServerHpComponentDefinition
        local hp_definition = {}
        hp_definition.m_hp = definition.m_hp.m_hp
        hp_definition.m_invincible_time = definition.m_hp.m_invincible_time

        ---@type ServerGameObjectDefinition
        local go_definition = {}
        go_definition.m_did = spawn_info.m_did
        go_definition.m_move_component_definition = move_definition
        go_definition.m_hp_component_definition = hp_definition
        go_definition.m_net_id = net_id
        script:initGameObject(go_definition)
    end

    return entity, script and script.m_gameobject
end

---@param self ServerCreation
---@param scene Scene
---@param player_related_definition PlayerRelatedDefinition
---@param object_definitions ObjectDefinitionTable
function _M:CreatePlayerHint(scene, player_related_definition, object_definitions)
end

---@param self ServerCreation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param object_definitions ObjectDefinitionTable
---@return LogicEntity, GameObject|nil
function _M:CreateItem(scene, spawn_info, position, object_definitions)
    TL_Common.GetContext():Log("server doesn't support spawning item ", spawn_info.m_did, " yet")
    return TL_Common.null_entity, nil
end

---@param self ServerCreation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param object_definitions ObjectDefinitionTable
---@return LogicEntity, GameObject|nil
function _M:CreateFX(scene, spawn_info, position, object_definitions)
    TL_Common.GetContext():Log("server doesn't support spawning fx ", spawn_info.m_did, " yet")
    return TL_Common.null_entity, nil
end

---@param self ServerCreation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param object_definitions ObjectDefinitionTable
---@return LogicEntity, GameObject|nil
function _M:CreateSkill(scene, spawn_info, position, object_definitions)
    TL_Common.GetContext():Log("server doesn't support spawning skill ", spawn_info.m_did, " yet")
    return TL_Common.null_entity, nil
end

return _M
