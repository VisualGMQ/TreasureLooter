#pragma once
#include "animation.hpp"
#include "common.hpp"
#include "level.hpp"

namespace tl {

class GameLevel: public Level {
public:
    using Level::Level;
    void Enter() override;
    void Update() override;
    void Quit() override;

private:
    GameObjectID playerGOID_;
    GameObjectID effectGOID_;

    void registTriggerCallback();
    void unregistTriggerCallback();
};

}
