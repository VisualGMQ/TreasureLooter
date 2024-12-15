#pragma once
#include "level.hpp"
#include "play_controller.hpp"

namespace tl {

class TestTriggerLevel : public Level {
public:
    using Level::Level;
    void Enter() override;
    void Update() override;
    
private:
    CommonMoveController controller_;
};

}  // namespace tl
