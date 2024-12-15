#pragma once
#include "gameobject.hpp"

namespace tl {

class Scene;

struct Anim {
    GameObject* go = nullptr;
    bool hideWhenFinish = true;
};

class AnimPool {
public:
    explicit AnimPool(Scene& scene);
    Anim& Create(const Vec2& pos, const std::string& animName, int loop, bool hideWhenFinish);
    void Update();
    
private:
    std::unordered_map<GameObjectID, Anim> anims_;
    std::vector<GameObjectID> cache_;
    Scene& scene_;
    GameObject* rootGO_ = nullptr;

    Anim& getUnusedAnim();
};

}
