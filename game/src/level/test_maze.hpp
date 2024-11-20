#pragma once
#include "level.hpp"

namespace tl {
class TestMazeLevel : public Level {
public:
    void Init() override;
    void Enter() override;
};
}
