#pragma once
#include "pch.hpp"
#include "id.hpp"

namespace tl {

enum class Flip {
    None = SDL_FLIP_NONE,
    Horizontal = SDL_FLIP_HORIZONTAL,
    Vertical = SDL_FLIP_VERTICAL,
};

uint32_t ParseFloat(std::string_view text, float* values, uint32_t count);
const char* GetXMLErrStr(tinyxml2::XMLError);

class GameObject;
class GameObjectManager;

using GameObjectID = ID<GameObject, GameObjectManager>;

using TimeType = uint64_t;

}