local World = require("@common/world")

local _M = {}

-- Collects all DIDs currently registered in the object definition table
-- (characters, items, fxs and skills).
local function collect_registered_dids(): {DID}
    local result = {}
    local definitions = World.GetInst().m_object_definitions
    if definitions then
        local seen = {}
        local function collect(did_table)
            for did in pairs(did_table) do
                if not seen[did] then
                    seen[did] = true
                    table.insert(result, did)
                end
            end
        end
        collect(definitions.m_characters)
        collect(definitions.m_items)
        collect(definitions.m_fxs)
        collect(definitions.m_skills)
    end
    return result
end

-- Maps a type name to a function converting a string argument into a Luau
-- value. Returns nil (and the wrapper logs) when the conversion fails.
local convert_param_fns = {
    number = function(text: string): number?
        return tonumber(text)
    end,

    string = function(text: string): string
        return text
    end,

    bool = function(text: string): boolean?
        if text == "true" then
            return true
        elseif text == "false" then
            return false
        end
        return nil
    end,

    DID = function(text: string): DID?
        local did = TL_Schema.DID.GetEnumFromName(text)
        if type(did) == "number" then
            for _, registered in collect_registered_dids() do
                if registered == did then
                    return did :: DID
                end
            end
        end
        return nil
    end,

    Entity = function(text: string): Entity?
        local num = string.match(text, "^Entity%<(%d+)%>$")
        if num then
            return tonumber(num)
        end
        return nil
    end,
}

local function collect_entities(scene: Scene, relationship_mgr: RelationshipManager): {Entity}
    local result = {}

    local function walk(entity: Entity)
        table.insert(result, entity)
        local relationship = relationship_mgr:Get(entity)
        if relationship then
            for i = 0, relationship:GetChildrenCount() - 1 do
                walk(relationship:Get(i))
            end
        end
    end

    walk(scene:GetRootEntity())
    return result
end

-- Maps a type name to a hint function. Each hint function takes no arguments
-- and returns a table of strings used as candidates in the debug panel.
local hint_fns = {
    number = function(): {string}
        return {}
    end,

    string = function(): {string}
        return {}
    end,

    bool = function(): {string}
        return { "true", "false" }
    end,

    DID = function(): {string}
        local result = {}
        for _, did in collect_registered_dids() do
            local name = TL_Schema.DID.GetEnumName(did)
            if name and name ~= "" then
                table.insert(result, name)
            end
        end
        table.sort(result)
        return result
    end,

    Entity = function(): {string}
        local ctx = TL_Client.GetContext()
        local scene = ctx:GetSceneManager():GetCurrentScene()
        if not scene then
            return {}
        end

        local result = {}
        for _, entity in collect_entities(scene, ctx:GetRelationshipManager()) do
            table.insert(result, "Entity<" .. tostring(entity) .. ">")
        end
        return result
    end,
}

-- Registers a debug command with parameter type information so the debug panel
-- can show param hints and validate the argument count.
function _M.RegisterDebugCmd(name: string, cmd_fn: (...any) -> (), param_types: {string})
    local hint_list = {}
    for _, type_name in param_types do
        local hint_fn = hint_fns[type_name]
        assert(hint_fn, "[debug_cmd_register] unknown param type '" .. tostring(type_name) .. "' for command '" .. name .. "'")
        table.insert(hint_list, hint_fn)
    end

    local wrapper = function(args: {string})
        local converted = {}
        for i = 1, #param_types do
            local convert_fn = convert_param_fns[param_types[i]]
            assert(convert_fn)

            local value = convert_fn(args[i] or "")
            if value == nil then
                TL_Client.GetContext():Log(
                    "[debug] " .. name .. ": can't convert param " .. tostring(i)
                        .. " '" .. (args[i] or "") .. "' to type '" .. param_types[i] .. "'"
                )
                return
            end
            table.insert(converted, value)
        end
        cmd_fn(table.unpack(converted))
    end

    TL_Client.GetContext():GetDebugPanel():RegisterCmd(name, wrapper, hint_list)
end

return _M
