#pragma once
#include "math.hpp"

namespace tl {

class GameObject;

struct Camera {
    Vec2 offset = Vec2::ZERO;
    Vec2 scale = Vec2::ONES;
    bool enable = false;

    operator bool() const noexcept;

    const Vec2& GetGlobalOffset() const;

    void Update(const GameObject& ownerGO);

private:
    Vec2 globalOffset_;
};

}  // namespace tl
