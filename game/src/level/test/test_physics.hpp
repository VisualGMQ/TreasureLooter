#pragma once
#include "level.hpp"
#include "gameobject.hpp"
#include "game_controller.hpp"

namespace tl {

class TestPhysicsLevel : public Level {
public:
    using Level::Level;
    void Init() {};
    void Enter();
    void Quit();
    void Update() {}
};

class RaycastController: public GameController {
public:
    void Update() override;

private:
    Vec2 hoverPoint_;
};


}  // namespace tl