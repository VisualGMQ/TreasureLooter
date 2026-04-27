#pragma once

// Keep all Lua/LuaBridge includes in one place so every translation
// unit sees identical Stack specializations and include order.
#include "common/entity.hpp"
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
struct luabridge::Stack<Entity>: public luabridge::Enum<Entity> {};

template <>
struct luabridge::Stack<TimerID>: public luabridge::Enum<TimerID> {};
