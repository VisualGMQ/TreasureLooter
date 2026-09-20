local Creation = require("common.creation")

---@class ServerCreation : Creation
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = Creation })

--- The server is authoritative: only it creates the level objects and then
--- tells clients to create their own copies (see World:AddSpawn).
_M.m_authoritative = true

local k_default_script = TL_Common.Path("scripts/server/gameobject_behavior.lua")

--- Notify the server world (which broadcasts to the connected clients) that an
--- object was spawned. Lazy require to avoid a require cycle with server.world.
---@param entity LogicEntity
---@param spawn_info ObjectSpawnDefinition
---@param net_id number
local function notifySpawn(entity, spawn_info, net_id)
    if entity == nil or entity == TL_Common.null_entity then
        return
    end
    local ServerWorld = require("server.world")
    ServerWorld.GetInst():AddSpawn(entity, spawn_info, net_id)
end

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

        ---@type ServerGameObjectDefinition
        local go_definition = {}
        go_definition.m_did = spawn_info.m_did
        go_definition.m_move_component_definition = move_definition
        go_definition.m_net_id = net_id
        script:initGameObject(go_definition)
    end

    notifySpawn(entity, spawn_info, net_id)

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
    local ctx = TL_Common.GetContext()

    local definition = object_definitions:GetItem(spawn_info.m_did)
    if not definition then
        ctx:Log("server spawn item failed: no definition for ", spawn_info.m_did)
        return TL_Common.null_entity, nil
    end

    local transform = TL_Common.Transform()
    transform.m_position = position

    local prefab = Creation.ConvertItemDefinitionToPrefab(definition)
    if not spawn_info.m_server_script:empty() then
        prefab.m_server_script = spawn_info.m_server_script
    end

    local entity = _M.CreatePrefab(self, scene, prefab, transform)
    ctx:Log("server spawn item ", spawn_info.m_did,
            " on SpawnPoint(", spawn_info.m_spawn_point_name, ")")
    notifySpawn(entity, spawn_info, 0)
    return entity, nil
end

---@param self ServerCreation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param object_definitions ObjectDefinitionTable
---@return LogicEntity, GameObject|nil
function _M:CreateFX(scene, spawn_info, position, object_definitions)
    local ctx = TL_Common.GetContext()

    local definition = object_definitions:GetFX(spawn_info.m_did)
    if not definition then
        ctx:Log("server spawn fx failed: no definition for ", spawn_info.m_did)
        return TL_Common.null_entity, nil
    end

    local transform = TL_Common.Transform()
    transform.m_position = position

    local prefab = Creation.ConvertFXDefinitionToPrefab(definition)
    if not spawn_info.m_server_script:empty() then
        prefab.m_server_script = spawn_info.m_server_script
    end

    local entity = _M.CreatePrefab(self, scene, prefab, transform)
    notifySpawn(entity, spawn_info, 0)
    return entity, nil
end

---@param self ServerCreation
---@param scene Scene
---@param spawn_info ObjectSpawnDefinition
---@param position Vec2
---@param object_definitions ObjectDefinitionTable
---@return LogicEntity, GameObject|nil
function _M:CreateSkill(scene, spawn_info, position, object_definitions)
    local ctx = TL_Common.GetContext()

    local definition = object_definitions:GetSkill(spawn_info.m_did)
    if not definition then
        ctx:Log("server spawn skill failed: no definition for ", spawn_info.m_did)
        return TL_Common.null_entity, nil
    end

    local transform = TL_Common.Transform()
    transform.m_position = position

    local prefab = Creation.ConvertSkillDefinitionToPrefab(definition)
    if not spawn_info.m_server_script:empty() then
        prefab.m_server_script = spawn_info.m_server_script
    end

    local entity = _M.CreatePrefab(self, scene, prefab, transform)
    notifySpawn(entity, spawn_info, 0)
    return entity, nil
end

return _M
