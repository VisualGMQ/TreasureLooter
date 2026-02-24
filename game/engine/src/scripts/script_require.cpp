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

namespace {

const char* kLoadedModulesKey = "tl_loaded_modules";

ScriptBinaryDataManager* s_require_mgr = nullptr;

static std::string moduleNameToPath(const std::string& modname)
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

static bool getCached(lua_State* L, const std::string& loadPath)
{
    lua_getfield(L, LUA_REGISTRYINDEX, kLoadedModulesKey);
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return false;
    }
    lua_pushlstring(L, loadPath.c_str(), loadPath.size());
    lua_gettable(L, -2);
    bool found = !lua_isnil(L, -1);
    if (found)
        lua_remove(L, -2); // 去掉 cache 表
    else
        lua_pop(L, 2);
    return found;
}

static void setCached(lua_State* L, const std::string& loadPath)
{
    lua_getfield(L, LUA_REGISTRYINDEX, kLoadedModulesKey);
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, kLoadedModulesKey);
    }
    // 栈: [..., module_result, cache_table]
    lua_pushlstring(L, loadPath.c_str(), loadPath.size());
    lua_pushvalue(L, -3); // 压入 module_result 的副本
    lua_settable(L, -3);  // cache_table[loadPath] = 副本，并弹出 key 与 value
    lua_pop(L, 1);        // 只弹出 cache_table，保留栈顶的 module_result
}

// 加载并执行一个 .luau 文件，结果留在栈顶，返回是否成功（失败时已推错误信息）
static bool loadAndRunModule(lua_State* L, ScriptBinaryDataManager* mgr, const std::string& loadPath)
{
    Path path(loadPath);
    auto handle = mgr->Load(path, false);
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
    int pcall_result = lua_pcall(L, 0, 1, 0);
    if (pcall_result != LUA_OK)
    {
        const char* err = lua_tostring(L, -1);
        lua_pushfstring(L, "error running '%s': %s", loadPath.c_str(), err ? err : "unknown");
        return false;
    }
    return true;
}

static int requireImpl(lua_State* L)
{
    if (!s_require_mgr)
    {
        luaL_error(L, "require: not initialized (call RegisterScriptBinaryDataManagerForRequire first)");
        return 0;
    }
    const char* modname = luaL_checkstring(L, 1);
    std::string file_path = moduleNameToPath(modname);
    if (file_path.empty())
    {
        luaL_error(L, "require: module not found '%s' (path: %s)", modname, file_path.c_str());
        return 0;
    }

    if (getCached(L, file_path))
        return 1;

    if (!loadAndRunModule(L, s_require_mgr, file_path))
    {
        const char* err = lua_tostring(L, -1);
        luaL_error(L, "require: %s", err ? err : "unknown error");
        return 0;
    }
    setCached(L, file_path);
    return 1;
}

} // namespace

void RegisterScriptBinaryDataManagerForRequire(lua_State* L, ScriptBinaryDataManager* mgr)
{
    (void)L;
    s_require_mgr = mgr;
}

void BindRequire(lua_State* L)
{
    TL_RETURN_IF_NULL_WITH_LOG(L, LOGE, "BindRequire: lua_State* is null");
    if (!s_require_mgr)
    {
        LOGE("BindRequire: call RegisterScriptBinaryDataManagerForRequire first");
        return;
    }
    lua_pushcfunction(L, requireImpl, "require");
    lua_setglobal(L, "require");
}
