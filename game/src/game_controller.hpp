#pragma once
#include "pch.hpp"
#include "gameobject.hpp"

namespace tl {

class GameController {
public:
    virtual void Update() = 0;
    virtual ~GameController() = default;
};

};  // namespace tl