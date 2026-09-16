#pragma once

// Keep all Lua/LuaBridge includes in one place so every translation
// unit sees identical Stack specializations and include order.
#include "common/entity.hpp"
#include "common/event.hpp"
#include "common/log.hpp"
#include "common/physics.hpp"
#include "common/tilemap.hpp"
#include "common/timer.hpp"

#include <tuple>
#include <utility>

// clang-format off
#include "lua.hpp"

#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/Array.h"
#include "LuaBridge/List.h"
#include "LuaBridge/Map.h"
#include "LuaBridge/Set.h"
#include "LuaBridge/UnorderedMap.h"
#include "LuaBridge/Vector.h"
// clang-format on

template <>
struct luabridge::Stack<LogicEntity> : public luabridge::Enum<LogicEntity> {};

template <>
struct luabridge::Stack<TimerID> : public luabridge::Enum<TimerID> {};

template <>
struct luabridge::Stack<EventListenerID>
    : public luabridge::Enum<EventListenerID> {};

template <>
struct luabridge::Stack<TilemapLayer::Type>
    : luabridge::Enum<TilemapLayer::Type, TilemapLayer::Type::Image,
                      TilemapLayer::Type::Object, TilemapLayer::Type::Tiled> {};

template <>
struct luabridge::Stack<TilemapObject::Type>
    : luabridge::Enum<TilemapObject::Type, TilemapObject::Type::None,
                      TilemapObject::Type::Point, TilemapObject::Type::Circle,
                      TilemapObject::Type::Rect, TilemapObject::Type::Polygon> {
};

template <>
struct luabridge::Stack<PhysicsStorageType>
    : luabridge::Enum<PhysicsStorageType, PhysicsStorageType::InChunk,
                      PhysicsStorageType::Normal> {};

template <>
struct luabridge::Stack<PhysicsShape::Type>
    : luabridge::Enum<PhysicsShape::Type, PhysicsShape::Type::Circle,
                      PhysicsShape::Type::Unknown, PhysicsShape::Type::Rect> {};

namespace tl {
// LuaBridge3 3.0-rc12: `LuaRef::call<R>` is ambiguous when `R == LuaRef`
// because the void overload of `callWithHandler` is not constrained on R.
// Pass the object type explicitly to select the R-returning overload.
template <class... Args>
luabridge::TypeResult<luabridge::LuaRef> CallLuaRef(
    const luabridge::LuaRef& fn, Args&&... args) {
    return luabridge::callWithHandler<luabridge::LuaRef, luabridge::LuaRef>(
        fn, std::ignore, std::forward<Args>(args)...);
}

// lua_pcall message handler. LuaBridge's default error handling drops the Lua
// error object and only reports "The lua function invocation raised an error",
// so log the real message (which contains `file:line`) plus a traceback whenever
// a Lua callback fails.
inline int LuaErrorHandler(lua_State* L) {
    const char* msg = lua_tostring(L, 1);
    if (msg == nullptr) {
        if (luaL_callmeta(L, 1, "__tostring") &&
            lua_type(L, -1) == LUA_TSTRING) {
            msg = lua_tostring(L, -1);
        } else {
            msg = lua_pushfstring(L, "(error object is a %s value)",
                                  luaL_typename(L, 1));
        }
    }
    luaL_traceback(L, L, msg, 1);
    LOGE("[Lua] {}", lua_tostring(L, -1));
    return 1;
}

// Calls a Lua function and logs the real Lua error + traceback on failure.
// Returns the LuaBridge result so callers can react if needed.
template <class... Args>
luabridge::TypeResult<void> CallLuaWithLog(const luabridge::LuaRef& fn,
                                           Args&&... args) {
    auto result = luabridge::callWithHandler<void>(
        fn, &LuaErrorHandler, std::forward<Args>(args)...);

    const auto lua_error =
        luabridge::makeErrorCode(luabridge::ErrorCode::LuaFunctionCallFailed);
    if (!result && result.error() != lua_error) {
        // Not a Lua-side raise (e.g. argument conversion failed), so
        // `LuaErrorHandler` did not log anything.
        LOGE("[Lua] call failed: {}", result.message());
    }
    return result;
}
}  // namespace tl
