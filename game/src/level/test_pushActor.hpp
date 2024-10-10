#pragma once
#include "level.hpp"
#include "game_controller.hpp"

namespace tl {

class TestPushActor: public Level {
public:
    void Init() override;
    void Enter() override;
    void Quit() override {}
    void Update() override;

private:
    Vec2 shooterPos_ = {300, 50};
    Vec2 shootDir_ = Vec2{1, 0};
    GameObject* box1_ = nullptr;
    GameObject* box2_ = nullptr;
};

}  // namespace tl