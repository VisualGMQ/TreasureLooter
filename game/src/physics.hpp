#pragma once
#include "pch.hpp"
#include "math.hpp"
#include "common.hpp"
#include "transform.hpp"

namespace tl {

class GameObject;

struct AABB {
    Vec2 center;
    Vec2 halfSize;

    operator bool() const { return halfSize.w > 0 && halfSize.h > 0; }
};

struct Shape {
    union {
        AABB aabb;
        Circle circle;
    };
    
    enum class Type {
        Unknown,
        AABB,
        Circle,
    } type = Type::Unknown;

    void SetCenter(const Vec2& c) {
        // NOTE: no security way for performance, may need check shape type
        aabb.center = c;
    }

    const Vec2& GetCenter() const {
        // NOTE: no security way for performance, may need check shape type
        return aabb.center;
    }

    Shape() : aabb{} {}
};

struct PhysicActor {
    friend class PhysicsScene;
    friend class Scene;

    enum class Type {
        Static,
        Dynamic,
    } type = Type::Dynamic;

    bool isTrigger = false;
    uint32_t filter = std::numeric_limits<uint32_t>::max();

    Shape shape;
    bool enable = false;

    operator bool() const { return enable; }

    void SetMovement(const Vec2&);

    const Shape& GetCollideShape() const { return collideShape_; }

private:
    Vec2 movement_;
    Shape collideShape_;

    std::vector<GameObjectID> enteredGOList_;
};

struct SweepHitInfo {
    PhysicActor* src = nullptr;
    PhysicActor* dst = nullptr;
    float t = -1;
    Vec2 normal;

    operator bool() const {
        return t >= 0;
    }
};

SweepHitInfo RaycastByAABB(const Vec2& start, const Vec2& dir,
                           const AABB&);

SweepHitInfo RaycastByCircle(const Vec2& start, const Vec2& dir,
                             const Circle& c);
SweepHitInfo RaycastByRoundAABB(const Vec2& start, const Vec2& dir,
                                float radius, const AABB&);

bool IsPointInRect(const Vec2&, const Rect&);
bool IsPointInAABB(const Vec2&, const AABB&);
bool IsPointInCircle(const Vec2&, const Circle&);
bool IsAABBOverlap(const AABB&, const AABB&);
bool IsCircleOverlap(const Circle&, const Circle&);
bool IsCircleAABBOverlap(const Circle&, const AABB&);

std::optional<std::tuple<float, float>> LinesIntersect(const Vec2& p1,
                                                       const Vec2& d1,
                                                       const Vec2& p2,
                                                       const Vec2& d2);

Vec2 AABBNearestPoint(const Vec2& p, const AABB&);
Vec2 CircleNearestPoint(const Vec2& p, const Circle&);

Shape GetShapeRelateBy(const GameObject&);
Shape GetShapeRelateBy(const Transform&, const PhysicActor&);

}  // namespace tl