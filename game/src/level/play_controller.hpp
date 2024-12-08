#pragma once
#include "common.hpp"
#include "game_controller.hpp"

namespace tl {

struct Bullet {
    float duration = 0;
    Vec2 dir;
    GameObject* go = nullptr;
};

class BulletPool {
public:
    Bullet& Create(const Vec2& pos, float duration, Animation* anim, Vec2 dir);
    void Update();

private:
    std::vector<Bullet> bullets_;
    std::vector<Bullet> cache_;
};

class PlayController : public GameController {
public:
    explicit PlayController(GameObjectID id);
    void Update();

private:
    GameObjectID id_;
    Vec2 dir_;
    BulletPool bulletPool_;

    void updateAnimation(const Vec2& axis);
};

}  // namespace tl
