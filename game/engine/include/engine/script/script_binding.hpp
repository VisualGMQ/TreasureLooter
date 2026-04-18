#pragma once
#include "engine/physics.hpp"
#include "engine/script/luabridge_include.hpp"
#include "engine/tilemap.hpp"
#include "lua.h"

// some enum register
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

void BindTLModule(lua_State* L);
