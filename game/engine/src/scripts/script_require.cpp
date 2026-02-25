#include "engine/script/script_require.hpp"
#include "engine/log.hpp"
#include "engine/macros.hpp"
#include "engine/path.hpp"
#include "engine/script/script.hpp"
#include "lua.h"
#include "lualib.h"
#include "luacode.h"

#include <cstring>
#include <string>

#include "engine/asset_manager.hpp"
#include "engine/context.hpp"

std::string moduleNameToPath(const std::string& modname)
{
    if (modname.empty())
        return "";
    std::string s;
    s.reserve(modname.size() + 8);
    for (size_t i = 0; i < modname.size(); ++i)
    {
        char c = modname[i];
        if (c == '.')
            s += '/';
        else
            s += c;
    }
    if (s.size() < 5 || s.compare(s.size() - 5, 5, ".luau") != 0)
        s += ".luau";
    return s;
}

bool GetCached(lua_State* L, const std::string& loadPath)
{
    lua_getfield(L, LUA_REGISTRYINDEX, kLoadedModulesKey.data());
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return false;
    }
    lua_pushlstring(L, loadPath.c_str(), loadPath.size());
    lua_gettable(L, -2);
    bool found = !lua_isnil(L, -1);
    if (found)
        lua_remove(L, -2);
    else
        lua_pop(L, 2);
    return found;
}

void SetCached(lua_State* L, const std::string& loadPath)
{
    lua_getfield(L, LUA_REGISTRYINDEX, kLoadedModulesKey.data());
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, kLoadedModulesKey.data());
    }
    // set module result to kLoadedModulesKey table
    lua_pushlstring(L, loadPath.c_str(), loadPath.size());
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    lua_pop(L, 1);
}

// load luau file, result leave on top of stack(or leave error when failed)
bool loadAndRunModule(lua_State* L, ScriptBinaryDataManager& mgr, const std::string& loadPath)
{
    Path path(loadPath);
    auto handle = mgr.Load(path, false);
    if (!handle)
    {
        lua_pushfstring(L, "module not found: '%s'", loadPath.c_str());
        return false;
    }
    const std::vector<char>& source = handle->GetContent();
    if (source.empty())
    {
        lua_pushfstring(L, "empty module: '%s'", loadPath.c_str());
        return false;
    }
    size_t bytecode_size = 0;
    char* bytecode = luau_compile(source.data(), source.size(), nullptr, &bytecode_size);
    if (!bytecode)
    {
        lua_pushfstring(L, "compile failed: '%s'", loadPath.c_str());
        return false;
    }
    int load_result = luau_load(L, loadPath.c_str(), bytecode, bytecode_size, 0);
    free(bytecode);
    if (load_result != 0)
    {
        const char* err = lua_tostring(L, -1);
        lua_pushfstring(L, "load failed: '%s': %s", loadPath.c_str(), err ? err : "unknown");
        return false;
    }

    // FIXME: maybe not need, luau_load executed code
    int pcall_result = lua_pcall(L, 0, 1, 0);
    if (pcall_result != LUA_OK)
    {
        const char* err = lua_tostring(L, -1);
        lua_pushfstring(L, "error running '%s': %s", loadPath.c_str(), err ? err : "unknown");
        return false;
    }
    
    return true;
}

int requireImpl(lua_State* L)
{
    auto& mgr = CURRENT_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>();
    
    const char* modname = luaL_checkstring(L, 1);
    std::string file_path = moduleNameToPath(modname);
    if (file_path.empty())
    {
        luaL_error(L, "require: module not found '%s' (path: %s)", modname, file_path.c_str());
        return 0;
    }

    if (GetCached(L, file_path))
        return 1;

    if (!loadAndRunModule(L, mgr, file_path))
    {
        const char* err = lua_tostring(L, -1);
        luaL_error(L, "require: %s", err ? err : "unknown error");
        return 0;
    }
    SetCached(L, file_path);
    return 1;
}

void BindRequire(lua_State* L)
{
    TL_RETURN_IF_NULL_WITH_LOG(L, LOGE, "BindRequire: lua_State* is null");
    lua_pushcfunction(L, requireImpl, "require");
    lua_setglobal(L, "require");
}
