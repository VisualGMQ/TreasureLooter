#pragma once

#include "engine/handle.hpp"
#include "engine/log.hpp"

#include "engine/script/luabridge_include.hpp"

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
    // function is at index 1, args start from index 2
    int nargs = lua_gettop(L) - 1;
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
				[t_class_name](Handle<T>& handle, const luabridge::LuaRef& key,
							   lua_State* L) -> luabridge::LuaRef {
					if (!handle) {
						return luabridge::LuaRef(L);
					}

					luabridge::LuaRef tl = luabridge::LuaRef::getGlobal(L, "TL");
					luabridge::LuaRef method = tl[t_class_name][key];
					if (!method.isNil() && method.isCallable()) {
						method.push(L);
						luabridge::lua_pushcclosure_x(L, &detail::HandleForwardCall<T>, 1);
						return luabridge::LuaRef::fromStack(L);
					}

					T* ptr = handle.Get();
					auto pushResult = luabridge::push(L, ptr);
					if (!pushResult) {
						LOGE("[Script] BindHandle __index push T* failed: {}",
							 pushResult.message());
						return luabridge::LuaRef(L);
					}
					luabridge::LuaRef obj = luabridge::LuaRef::fromStack(L, -1);
					luabridge::LuaRef value = obj[key];
					lua_pop(L, 1);
					return value;
				})
			.addNewIndexMetaMethod(
				+[](Handle<T>& handle, const luabridge::LuaRef& key,
					const luabridge::LuaRef& value,
					lua_State* L) -> luabridge::LuaRef {
					if (!handle) {
						return luabridge::LuaRef(L);
					}

					T* ptr = handle.Get();
					auto pushResult = luabridge::push(L, ptr);
					if (!pushResult) {
						LOGE("[Script] BindHandle __newindex push T* failed: {}",
							 pushResult.message());
						return luabridge::LuaRef(L);
					}
					luabridge::LuaRef obj = luabridge::LuaRef::fromStack(L, -1);
					obj[key] = value;
					lua_pop(L, 1);
					return luabridge::LuaRef(L);
				})
			.endClass()
			.endNamespace();
}
