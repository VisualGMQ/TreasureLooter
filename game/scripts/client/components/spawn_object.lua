local SpawnObjectComponent = require("common.components.spawn_object")
local World = require("common.world")

---@class ClientSpawnObjectComponentDefinition : SpawnObjectComponentDefinition

---@class ClientSpawnObjectComponent : SpawnObjectComponent
local _M = {}
_M.__index = _M
setmetatable(_M, { __index = SpawnObjectComponent })

local k_default_script = TL_Common.Path("scripts/client/gameobject_behavior.lua")
local k_spawn_radius = 16

---@param gameobject any
---@param definition ClientSpawnObjectComponentDefinition
---@return ClientSpawnObjectComponent
function _M.new(gameobject, definition)
    local self = SpawnObjectComponent.new(gameobject, definition)
    ---@cast self ClientSpawnObjectComponent
    return setmetatable(self, _M)
end

---@param mine_position Vec2
function _M:_spawnOne(mine_position)
    local ClientCreation = require("client.creation")
    local ClientSpawnedComponent = require("client.components.spawned")

    local ctx = TL_Client.GetContext()
    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        return
    end

    local world = World.GetInst()
    if not world.m_object_definitions then
        return
    end

    local mine_entity = self._gameobject:GetEntity()

    local angle = math.random() * math.pi * 2
    local radius = math.random() * k_spawn_radius
    local target_x = math.cos(angle) * radius

    local definition = world.m_object_definitions:GetItem(self._object_did)
    local prefab = ClientCreation.ConvertItemDefinitionToPrefab(definition)
    prefab.m_client_script = k_default_script

    if #self._spawn_animations > 0 then
        local multi_anim = prefab.m_animations
        if not multi_anim then
            multi_anim = TL_Schema.MultiAnimationPlayerDefinition()
        end
        local anims = multi_anim.m_animations
        for _, anim in ipairs(self._spawn_animations) do
            local anim_player_def = TL_Schema.AnimationPlayerDefinition()
            anim_player_def.m_animation = anim
            anim_player_def.m_auto_play = false
            anim_player_def.m_loop = 0
            anim_player_def.m_rate = 1.0
            table.insert(anims, anim_player_def)
        end
        multi_anim.m_animations = anims
        prefab.m_animations = multi_anim
    end

    local controller_entity = self:borrowController(ctx, scene, mine_entity)

    local item_transform = TL_Common.Transform()
    item_transform.m_position = TL_Common.Vec2(0, 0)

    local entity = scene:Instantiate(prefab, item_transform)
    ctx:GetAssetsManager():GetPrefabManager():Unload(prefab)

    local ctl_relationship = ctx:GetRelationshipManager():Get(controller_entity)
    if ctl_relationship then
        local item_relationship = ctx:GetRelationshipManager():Get(entity)
        if item_relationship then
            item_relationship:RemoveFromParent()
        end
        ctl_relationship:AddChild(entity)
    end

    local script = ctx:GetScriptManager():Get(entity)
    if script then
        local go_definition = ClientCreation.ConvertItemDefinitionToGODefinition(entity, self._object_did, definition)
        script:initGameObject(go_definition)
    end

    local spawn_anim_indices = {}
    local anim_mgr = ctx:GetAnimationPlayerManager()
    if anim_mgr:Has(entity) and #self._spawn_animations > 0 then
        local total = anim_mgr:GetComponentSize(entity)
        local start_idx = total - #self._spawn_animations
        for j = 0, #self._spawn_animations - 1 do
            table.insert(spawn_anim_indices, start_idx + j)
        end
    end

    local mineral_gameobject = script.m_gameobject
    local spawner = ClientSpawnedComponent.new(mineral_gameobject, self._gameobject, controller_entity, target_x, spawn_anim_indices)
    mineral_gameobject.m_spawned_component = spawner

    if anim_mgr:Has(entity) then
        for _, idx in ipairs(spawn_anim_indices) do
            local anim = anim_mgr:Get(entity, idx)
            if anim then
                anim:Play()
            end
        end
    end
end

return _M
