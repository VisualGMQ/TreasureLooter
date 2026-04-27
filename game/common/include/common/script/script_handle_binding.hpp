#pragma once

#include "common/handle.hpp"
#include "common/log.hpp"

#include "common/script/luabridge_include.hpp"

#include <string>

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
    using handle_type = Handle<T>;
    luabridge::getGlobalNamespace(L)
           .beginNamespace("TL").beginNamespace("Common")
			.beginClass<handle_type>(name.c_str())
			.addFunction(
				"IsValid",
				+[](handle_type handle) { return static_cast<bool>(handle); })
			.addFunction("GetFilename", &handle_type::GetFilename)
            .addFunction("GetUUID", +[](const handle_type& h) -> UUID { return h.GetUUID(); })
			.addIndexMetaMethod(
				[t_class_name](handle_type& handle, const luabridge::LuaRef& key,
							   lua_State* L) -> luabridge::LuaRef {
					if (!handle) {
						return luabridge::LuaRef(L);
					}

					luabridge::LuaRef tl = luabridge::LuaRef::getGlobal(L, "TL");
					// Engine: TL.Common; schema: TL.Schema (legacy TL root kept for older builds).
					luabridge::LuaRef cls = tl["Common"][t_class_name];
					if (cls.isNil()) {
						cls = tl["Schema"][t_class_name];
					}
					if (cls.isNil()) {
						cls = tl[t_class_name];
					}
					luabridge::LuaRef method = cls[key];
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
				+[](handle_type& handle, const luabridge::LuaRef& key,
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
			.endNamespace().endNamespace();
}

