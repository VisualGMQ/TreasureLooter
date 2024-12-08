#pragma once
#include "level.hpp"

namespace tl {
class TestMazeLevel : public Level {
public:
    using Level::Level;
    void Init() override;
    void Enter() override;
};
}
