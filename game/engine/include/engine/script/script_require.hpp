#pragma once

#include "lua.h"
#include <string>

constexpr std::string_view kLoadedModulesKey = "TLLoadModules";

void BindRequire(lua_State* L);

/**
 * Get cached module from lua
 * If module cached, put module on stack then return true, else get false
 */
bool GetCached(lua_State* L, const std::string& loadPath);

void SetCached(lua_State* L, const std::string& loadPath);
