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

size_t Sweep(const Shape& shape, const Vec2& dir, float distance, SweepHitInfo*, size_t maxHitInfoCount);

size_t Raycast(const Vec2& start, const Vec2& dir, SweepHitInfo*, size_t maxHitInfoCount);

SweepHitInfo RaycastByRect(const Vec2& start, const Vec2& dir,
                           const Rect& rect);

SweepHitInfo RaycastByCircle(const Vec2& start, const Vec2& dir,
                             const Circle& c);
SweepHitInfo RaycastByRoundRect(const Vec2& start, const Vec2& dir,
                                float radius, const Rect& rect);

bool IsPointInRect(const Vec2& p, const Rect& rect);
bool IsPointInCircle(const Vec2& p, const Circle& circle);
bool IsRectsIntersect(const Rect& r1, const Rect& r2);
bool IsCircleIntersect(const Circle& c1, const Circle& c2);
bool IsCircleRectIntersect(const Circle&, const Rect&);

std::optional<std::tuple<float, float>> LinesIntersect(const Vec2& p1,
                                                       const Vec2& d1,
                                                       const Vec2& p2,
                                                       const Vec2& d2);

Vec2 RectNearestPoint(const Vec2& p, const Rect&);
Vec2 CircleNearestPoint(const Vec2& p, const Circle&);

}  // namespace tl