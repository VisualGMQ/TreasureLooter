#pragma once
#include "pch.hpp"
#include "id.hpp"

namespace tl {

uint32_t ParseFloat(std::string_view text, float* values, uint32_t count);

class GameObject;
class GameObjectManager;

using GameObjectID = ID<GameObject, GameObjectManager>;

using TimeType = uint64_t;

}