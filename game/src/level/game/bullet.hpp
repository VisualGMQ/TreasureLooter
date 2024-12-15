#pragma once
#include "common.hpp"
#include "math.hpp"
#include "scene.hpp"

namespace tl {

struct Bullet {
    float duration = 0;
    Vec2 dir;
    GameObject* go = nullptr;
};

class BulletPool {
public:
    explicit BulletPool(Scene& scene);
    Bullet& Create(const Vec2& pos, float duration, Animation* anim, Vec2 dir);
    void Update();

private:
    std::unordered_map<GameObjectID, Bullet> bullets_;
    std::vector<GameObjectID> cache_;
    Scene& scene_;
    GameObject* bulletRootGO_ = nullptr;
};

}
