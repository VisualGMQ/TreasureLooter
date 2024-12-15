#pragma once
#include "level.hpp"
#include "play_controller.hpp"

namespace tl {
class TestMazeLevel : public Level {
public:
    using Level::Level;
    void Init() override;
    void Enter() override;
    void Update() override;
    
private:
    CommonMoveController controller_;
};

}
