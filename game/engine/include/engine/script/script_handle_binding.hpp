#pragma once

#include "lua.h"
#include "lualib.h"
#include "LuaBridge/LuaBridge.h"
#include "engine/handle.hpp"
#include "engine/log.hpp"

namespace detail {

template <typename T>
int HandleForwardCall(lua_State* L) {
    auto handleResult = luabridge::get<Handle<T>>(L, 1);
    if (!handleResult)
        return 0;
    Handle<T> h = *handleResult;
    if (!h) {
        lua_pushnil(L);
        return 1;
    }
    T* ptr = h.Get();
    auto pushResult = luabridge::push(L, ptr);
    if (!pushResult) {
        LOGE("[Script] HandleForwardCall: push T* failed: {}", pushResult.message());
        return 0;
    }
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_remove(L, 1);
    lua_insert(L, 1);
    lua_insert(L, 2);
    int nargs = lua_gettop(L);
    lua_call(L, nargs, LUA_MULTRET);
    return lua_gettop(L);
}

}  // namespace detail

template <typename T>
void BindHandle(const std::string& name,
                lua_State* L,
                const char* t_class_name) {
    luabridge::getGlobalNamespace(L)
           .beginNamespace("TL")
			.beginClass<Handle<T>>(name.c_str())
			.addFunction(
				"IsValid",
				+[](Handle<T> handle) { return static_cast<bool>(handle); })
			.addIndexMetaMethod(
				[t_class_name](Handle<T>&, const luabridge::LuaRef& key,
							   lua_State* L) -> luabridge::LuaRef {
					luabridge::LuaRef tl = luabridge::LuaRef::getGlobal(L, "TL");
					luabridge::LuaRef method = tl[t_class_name][key];
					if (method.isNil() || !method.isCallable())
						return luabridge::LuaRef(L);
					method.push(L);
					luabridge::lua_pushcclosure_x(L, &detail::HandleForwardCall<T>, 1);
					return luabridge::LuaRef::fromStack(L);
				})
			.endClass()
			.endNamespace();
}
