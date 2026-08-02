#pragma once

#include "lua.h"
#include <string>
#include <unordered_map>

#include "common/path.hpp"

class ScriptBinaryDataManager;

class LuauRequireContext {
public:
    static constexpr std::string_view kLoadedModulesKey = "TLLoadModules";

    static void InitModuleRegisterTable(lua_State* L);

    /**
     * Get cached module from lua
     * If module cached, put module on stack then return true, else get false
     */
    static bool GetCached(lua_State* L, const std::string& loadPath);

    static void SetCached(lua_State* L, const std::string& loadPath);

    static void BindRequire(lua_State* L);

    void RegisterAliasPath(const std::string& name, const Path& path);
    void UnregisterAliasPath(const std::string& name);
    [[nodiscard]] Path FindAliasPath(const std::string& name) const;
    bool PathToModuleName(const Path& file_path, std::string& out_modname) const;

private:
    std::unordered_map<std::string, Path> m_alias_paths;

    static int requireImpl(lua_State* L);
    static bool loadAndRunModule(lua_State* L, ScriptBinaryDataManager& mgr,
                                 const std::string& loadPath);
    bool moduleNameToPath(const std::string& modname, Path& out_path);
};
