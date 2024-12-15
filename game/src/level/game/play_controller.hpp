#pragma once
#include "anim_pool.hpp"
#include "bullet.hpp"
#include "common.hpp"
#include "math.hpp"
#include "scene.hpp"

namespace tl {

class GameLevel;

class PlayController {
public:
    explicit PlayController(Scene& scene, BulletPool& bulletPool, AnimPool& animPool);
    void SetPlayer(GameObjectID id);
    void Update();

private:
    GameObjectID id_;
    Vec2 dir_;
    Scene& scene_;
    BulletPool& bulletPool_;
    AnimPool& animPool_;

    void updateAnimation(const Vec2& axis);
};

}  // namespace tl
