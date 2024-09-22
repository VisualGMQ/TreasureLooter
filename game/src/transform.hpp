#pragma once
#include "math.hpp"

namespace tl {

struct Transform {
    Vec2 position;
    Vec2 scale = Vec2::ONES;
    float rotation = 0;  // in degrees
};

Transform CalcTransformFromParent(const Transform& parentGlobalTransform,
                                  const Transform& localTransform);

}  // namespace tl
