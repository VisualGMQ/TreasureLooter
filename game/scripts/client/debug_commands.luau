local ClientCreation = require("@client/creation")
local ClientWorld = require("@client/world")
local World = require("@common/world")

local _M = {}

-- Parse [did_num, x, y] string args. Returns the resolved DID and a world-space
-- Vec2 position, or (nil, nil) on any error (after logging).
local function parse_spawn_args(name: string, did_prefix: string, args: {string}): (DID?, Vec2?)
    local ctx = TL_Client.GetContext()

    if #args < 3 then
        ctx:Log("[debug] " .. name .. " usage: " .. name .. " <did> <x> <y>")
        return nil, nil
    end

    local num = tonumber(args[1])
    local x = tonumber(args[2])
    local y = tonumber(args[3])
    if not num or not x or not y then
        ctx:Log("[debug] " .. name .. ": arguments must be numbers")
        return nil, nil
    end

    local did = (TL_Schema.DID :: any)[did_prefix .. tostring(math.floor(num))]
    if not did then
        ctx:Log("[debug] " .. name .. ": invalid " .. did_prefix .. " number ", num)
        return nil, nil
    end

    return did, TL_Common.Vec2(x, y)
end

-- Spawn an object through a Creation.CreateXXX function, then parent it under
-- the land layer (fall back to scene root) so it lives in the render hierarchy.
local function spawn_via_creation(name: string, did_prefix: string, create_fn, args: {string})
    local ctx = TL_Client.GetContext()

    local did, position = parse_spawn_args(name, did_prefix, args)
    if not did or not position then
        return
    end

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

local function spawn_fx(args: {string})
    local did, position = parse_spawn_args("spawn.fx", "FXDID", args)
    if not did or not position then
        return
    end

    local world = ClientWorld.GetInst() :: ClientWorld.ClientWorld
    world:AddFX(did, position)

    TL_Client.GetContext():Log("[debug] spawn.fx: spawned fx at ", position)
end

function _M.RegisterAllDebugCommand()
    local panel = TL_Client.GetContext():GetDebugPanel()
    panel:RegisterCmd("spawn.item", function(args: {string})
        spawn_via_creation("spawn.item", "ItemDID", ClientCreation.CreateItem, args)
    end)
    panel:RegisterCmd("spawn.skill", function(args: {string})
        spawn_via_creation("spawn.skill", "SkillDID", ClientCreation.CreateSkill, args)
    end)
    panel:RegisterCmd("spawn.fx", spawn_fx)
end

return _M
