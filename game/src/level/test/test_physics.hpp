#pragma once
#include "level.hpp"
#include "gameobject.hpp"

namespace tl {

class TestPhysicsLevel : public Level {
public:
    using Level::Level;
    void Enter() override;
    void Quit() override;
    void Update() override;

private:
    Vec2 hoverPoint_;
};


}  // namespace tl