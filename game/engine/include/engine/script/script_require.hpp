#pragma once

#include "lua.h"

class ScriptBinaryDataManager;

/** 注册 ScriptBinaryDataManager 供 require 使用，须在 BindTLModule 之后、BindRequire 之前调用 */
void RegisterScriptBinaryDataManagerForRequire(lua_State* L, ScriptBinaryDataManager* mgr);

/** 设置全局 require 与 package.loaded，须在 RegisterScriptBinaryDataManagerForRequire 之后调用 */
void BindRequire(lua_State* L);
