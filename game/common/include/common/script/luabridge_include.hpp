#pragma once

// Keep all Lua/LuaBridge includes in one place so every translation
// unit sees identical Stack specializations and include order.
#include "common/entity.hpp"
#include "common/event.hpp"
#include "common/physics.hpp"
#include "common/tilemap.hpp"
#include "common/timer.hpp"

// clang-format off
#include "lua.h"
#include "lualib.h"
#include "luacode.h"
#include "luaconf.h"

#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/Array.h"
#include "LuaBridge/List.h"
#include "LuaBridge/Map.h"
#include "LuaBridge/Set.h"
#include "LuaBridge/UnorderedMap.h"
#include "LuaBridge/Vector.h"
// clang-format on

template <>
struct luabridge::Stack<Entity> : public luabridge::Enum<Entity> {};

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
