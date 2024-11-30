#pragma once
#include "game_controller.hpp"
#include "level.hpp"

namespace tl {

class TestPushActor : public Level {
public:
    using Level::Level;
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