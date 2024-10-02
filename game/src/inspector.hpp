#pragma once
#include "pch.hpp"
#include "gameobject.hpp"

namespace tl {

class Inspector {
public:
    void Update();

private:
    std::string lastGOName_;

    void updateName(GameObject& go);
    void updateTransform(GameObject& go);
    void updateSprite(GameObject& go);
    void updateAnimator(GameObject& go);
    void updateTransformGeneric(Transform& transform);
};

}
