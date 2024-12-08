#pragma once
#include "math.hpp"

namespace tl {

struct Transform {
    Vec2 position;
    Vec2 scale = Vec2::ONES;
    float rotation = 0;  // in degrees

    bool operator==(const Transform& o) const noexcept;
    bool operator!=(const Transform& o) const noexcept;
};

Transform CalcTransformFromParent(const Transform& parentGlobalTransform,
                                  const Transform& localTransform);

Transform CalcLocalTransformToParent(const Transform& parentGlobalTransform,
                                  const Transform& globalTrans);

}  // namespace tl
