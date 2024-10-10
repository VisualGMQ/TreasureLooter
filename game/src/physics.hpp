#pragma once
#include "pch.hpp"
#include "math.hpp"

namespace tl {

struct Shape {
    union {
        Rect rect = {};  // use center & halfSize
        Circle circle;
    };
    enum class Type {
        Unknown,
        Rect,
        Circle,
    } type = Type::Unknown;
};

struct PhysicsActor {
    Shape shape;
    bool enable = false;

    operator bool() const { return enable; }
};

struct SweepHitInfo {
    const Shape* src = nullptr;
    const Shape* dst = nullptr;
    float t1 = -1;
    float t2 = -1;
    bool isInitialOverlap = false;
};

size_t Sweep(const Shape& shape, const Vec2& dir, float distance);

size_t Raycast(const Vec2& start, const Vec2& dir, SweepHitInfo*, size_t maxHitInfoCount);

SweepHitInfo RaycastByRect(const Vec2& start, const Vec2& dir,
                           const Rect& rect);

SweepHitInfo RaycastByCircle(const Vec2& start, const Vec2& dir,
                             const Circle& c);

bool IsPointInRect(const Vec2& p, const Rect& rect);

bool IsPointInCircle(const Vec2& p, const Circle& circle);

std::optional<std::tuple<float, float>> LinesIntersect(const Vec2& p1,
                                                       const Vec2& d1,
                                                       const Vec2& p2,
                                                       const Vec2& d2);

}  // namespace tl