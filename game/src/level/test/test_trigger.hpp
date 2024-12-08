#pragma once
#include "level.hpp"

namespace tl {

class TestTriggerLevel : public Level {
public:
    using Level::Level;
    void Enter();
};

}  // namespace tl
