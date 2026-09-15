local ClientCreation = require("client.creation")
local ClientWorld = require("client.world")
local World = require("common.world")
local DebugCmdRegister = require("client.debug_cmd_register")

---@class DebugCommands
local _M = {}

-- Spawn an object through a Creation.CreateXXX function, then parent it under
-- the land layer (fall back to scene root) so it lives in the render hierarchy.
---@param name string
---@param did DID
---@param x number
---@param y number
---@param create_fn function
local function spawn_via_creation(name, did, x, y, create_fn)
    local ctx = TL_Client.GetContext()

    local scene = ctx:GetSceneManager():GetCurrentScene()
    if not scene then
        ctx:Log("[debug] " .. name .. ": no active scene")
        return
    end

    local object_definitions = World.GetInst().m_object_definitions
    if not object_definitions then
        ctx:Log("[debug] " .. name .. ": object definitions not loaded")
        return
    end

    local relationship_mgr = ctx:GetRelationshipManager()

    local parent = World.GetInst().m_land_entity
    if not relationship_mgr:Get(parent) then
        parent = scene:GetRootEntity()
    end

    -- Treat the given x/y as a world position: convert to the parent's local
    -- space before spawning.
    local position = TL_Common.Vec2(x, y)
    local local_pos = position
    local parent_transform = ctx:GetTransformManager():Get(parent)
    if parent_transform then
        local_pos = position - parent_transform:GetGlobalPosition()
    end

    local spawn_info = TL_Schema.ObjectSpawnDefinition()
    spawn_info.m_did = did

    local entity = create_fn(ClientCreation, scene, spawn_info, local_pos, object_definitions)

    local parent_relationship = relationship_mgr:Get(parent)
    local entity_relationship = relationship_mgr:Get(entity)
    if parent_relationship and entity_relationship then
        entity_relationship:RemoveFromParent()
        parent_relationship:AddChild(entity)
    end

    ctx:Log("[debug] " .. name .. ": spawned entity ", entity, " at ", position)
end

function _M.RegisterAllDebugCommand()
    DebugCmdRegister.RegisterDebugCmd("spawn.item", function(did, x, y)
        spawn_via_creation("spawn.item", did, x, y, ClientCreation.CreateItem)
    end, { "DID", "number", "number" })

    DebugCmdRegister.RegisterDebugCmd("spawn.skill", function(did, x, y)
        spawn_via_creation("spawn.skill", did, x, y, ClientCreation.CreateSkill)
    end, { "DID", "number", "number" })

    DebugCmdRegister.RegisterDebugCmd("spawn.fx", function(did, x, y)
        local world = ClientWorld.GetInst()
        world:AddFX(did, TL_Common.Vec2(x, y))

        TL_Client.GetContext():Log("[debug] spawn.fx: spawned fx at ", TL_Common.Vec2(x, y))
    end, { "DID", "number", "number" })
end

return _M
