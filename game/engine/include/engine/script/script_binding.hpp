#pragma once

struct lua_State;

/** 将 TL 引擎模块绑定到 Luau VM（使用 LuaBridge3）。在 ScriptBinaryDataManager::bindModule() 中调用。 */
void BindTLModule(lua_State* L);
