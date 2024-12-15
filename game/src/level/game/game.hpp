#pragma once
#include "anim_pool.hpp"
#include "animation.hpp"
#include "common.hpp"
#include "level.hpp"
#include "play_controller.hpp"

namespace tl {

class GameLevel: public Level {
public:
    GameLevel(Scene& scene);
    void Init() override;
    void Enter() override;
    void Update() override;
    void Quit() override;

private:
    std::vector<GameObjectID> roles_;
    GameObjectID playerGOID_;
    BulletPool bulletPool_;
    AnimPool animPool_;
    PlayController controller_;

    void registTriggerCallback();
    void registStickCallback();
    void updateCarray();
    void clearStickState();
};

}
