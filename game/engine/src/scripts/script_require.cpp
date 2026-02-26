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

bool LuauRequireContext::moduleNameToPath(const std::string& modname, Path& out_path)
{
    TL_RETURN_VALUE_IF_FALSE(!modname.empty(), false);

    // normalize module path
    std::string path = modname;
    std::replace(path.begin(), path.end(), '\\', '/');
    
    // alias path
    Path alias_path;
    if (path.size() > 1 && path[0] == '@') {
        auto idx = path.find_first_of('/');
        TL_RETURN_VALUE_IF_FALSE(idx != std::string::npos, false);
        
        auto alias = path.substr(0, idx);
        alias_path = FindAliasPath(alias);
        TL_RETURN_VALUE_IF_FALSE_WITH_LOG(!alias_path.empty(), false, LOGE, "[Luau]: can't find require alias {}", alias);

        path = path.substr(idx + 1);
    }

    out_path = alias_path / path;
    out_path.replace_extension(".luau");
    return true;
}

void LuauRequireContext::InitModuleRegisterTable(lua_State* L) {
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LuauRequireContext::kLoadedModulesKey.data());
}

bool LuauRequireContext::GetCached(lua_State* L, const std::string& loadPath)
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

void LuauRequireContext::SetCached(lua_State* L, const std::string& loadPath)
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
bool LuauRequireContext::loadAndRunModule(lua_State* L, ScriptBinaryDataManager& mgr, const std::string& loadPath)
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

void LuauRequireContext::UnregisterAliasPath(const std::string& name) {
    m_alias_paths.erase(name);
}

Path LuauRequireContext::FindAliasPath(const std::string& name) const {
    if (auto it = m_alias_paths.find(name); it != m_alias_paths.end()) {
        return it->second;
    }
    return {};
}

int LuauRequireContext::requireImpl(lua_State* L)
{
    auto& script_binary_data_manager = CURRENT_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>();
    auto& require_ctx = script_binary_data_manager.GetRequireContext();
    
    const char* modname = luaL_checkstring(L, 1);
    Path module_path;
    bool success = require_ctx.moduleNameToPath(modname, module_path);
    if (!success) {
        luaL_error(L, "require: module parse failed", modname);
        return 0;
    }

    std::string file_path = module_path.string();
    if (GetCached(L, file_path))
        return 1;

    if (!loadAndRunModule(L, script_binary_data_manager, file_path))
    {
        const char* err = lua_tostring(L, -1);
        luaL_error(L, "require: %s", err ? err : "unknown error");
        return 0;
    }
    SetCached(L, file_path);
    return 1;
}

void LuauRequireContext::BindRequire(lua_State* L)
{
    TL_RETURN_IF_NULL_WITH_LOG(L, LOGE, "BindRequire: lua_State* is null");
    lua_pushcfunction(L, LuauRequireContext::requireImpl, "require");
    lua_setglobal(L, "require");
}

void LuauRequireContext::RegisterAliasPath(const std::string& name,
    const Path& path) {
    m_alias_paths["@" + name] = path;
}
