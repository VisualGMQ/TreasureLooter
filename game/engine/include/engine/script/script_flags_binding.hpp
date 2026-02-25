#pragma once

#include "engine/flag.hpp"
#include "lua.h"
#include "lualib.h"
#include "LuaBridge/LuaBridge.h"
#include <type_traits>

template <typename T>
struct luabridge::Stack<Flags<T>> {
    using underlying_type = typename Flags<T>::underlying_type;

    [[nodiscard]] static luabridge::Result push(lua_State* L, const Flags<T>& value) {
#if LUABRIDGE_SAFE_STACK_CHECKS
        if (!lua_checkstack(L, 1))
            return luabridge::makeErrorCode(luabridge::ErrorCode::LuaStackOverflow);
#endif
        lua_pushinteger(L, static_cast<lua_Integer>(value.Value()));
        return {};
    }

    [[nodiscard]] static luabridge::TypeResult<Flags<T>> get(lua_State* L, int index) {
        if (lua_type(L, index) != LUA_TNUMBER)
            return luabridge::makeErrorCode(luabridge::ErrorCode::InvalidTypeCast);
        auto v = static_cast<typename Flags<T>::underlying_type>(lua_tointeger(L, index));
        return Flags<T>(v);
    }

    [[nodiscard]] static bool isInstance(lua_State* L, int index) {
        return lua_type(L, index) == LUA_TNUMBER;
    }
};