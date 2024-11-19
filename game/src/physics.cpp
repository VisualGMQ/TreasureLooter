#include "physics.hpp"
#include "context.hpp"
#include "log.hpp"
#include "macro.hpp"
#include "scene.hpp"

namespace tl {

void PhysicActor::SetMovement(const Vec2& v) {
    if (type == Type::Dynamic) {
        movement_ = v;
    }
}

size_t PhysicsScene::Raycast(const Vec2& start, const Vec2& dir,
                             SweepHitInfo* outInfo, size_t maxHitInfoCount) {
    size_t resultCount = 0;
    
    AABB aabb1;
    aabb1.center = start + dir * 0.5;
    Vec2 halfDir = dir * 0.5;  
    aabb1.halfSize.x = std::abs(halfDir.x);
    aabb1.halfSize.y = std::abs(halfDir.y);
    
    for (auto act : actors_) {
        PhysicActor* actor = act.actor;
        
        TL_BREAK_IF_FALSE(resultCount < maxHitInfoCount);
        TL_CONTINUE_IF_FALSE(quickCheckNeedSweep(aabb1, actor->collideShape_.aabb, dir));

        if (actor->shape.type == Shape::Type::AABB) {
            SweepHitInfo info = RaycastByAABB(start, dir, actor->collideShape_.aabb);
            TL_CONTINUE_IF_FALSE(info.t >= 0);
            info.dst = actor;
            outInfo[resultCount++] = std::move(info);
        } else if (actor->shape.type == Shape::Type::Circle) {
            SweepHitInfo info =
                RaycastByCircle(start, dir, actor->collideShape_.circle);
            TL_CONTINUE_IF_FALSE(info.t >= 0);
            info.dst = actor;
            outInfo[resultCount++] = std::move(info);
        }
    }

    return resultCount;
}

size_t PhysicsScene::Sweep(const Shape& shape, const Vec2& dir,
                           SweepHitInfo* hitInfos, size_t maxHitInfoCount) {
    size_t count = 0;
    for (auto& other : actors_) {
        TL_RETURN_VALUE_IF_FALSE(count <= maxHitInfoCount, count);

        SweepHitInfo hit = sweep(shape, other.actor->collideShape_, dir);
        if (hit.t > 0) {
            hitInfos[count++] = hit;
        }
    }

    return count;
}

SweepHitInfo RaycastByAABB(const Vec2& start, const Vec2& dir,
                           const AABB& aabb) {
    SweepHitInfo info;
    if (IsPointInAABB(start, aabb)) {
        return info;
    }

    const Vec2 pts[] = {
        aabb.center - aabb.halfSize,
        {aabb.center.x + aabb.halfSize.w, aabb.center.y - aabb.halfSize.h},
        aabb.center + aabb.halfSize,
        {aabb.center.x - aabb.halfSize.w, aabb.center.y + aabb.halfSize.h},
    };

    const float len[] = {
        aabb.halfSize.w * 2,
        aabb.halfSize.h * 2,
        aabb.halfSize.w * 2,
        aabb.halfSize.h * 2,
    };

    const Vec2 normal[] = {
        { 0, -1},
        { 1,  0},
        { 0,  1},
        {-1,  0},
    };

    int intersectCount = 0;
    info.t = std::numeric_limits<float>::max();
    for (int i = 0; i < 4; i++) {
        auto result =
            LinesIntersect(start, dir, pts[i], pts[(i + 1) % 4] - pts[i]);
        TL_CONTINUE_IF_FALSE(result);

        intersectCount++;

        auto [t1, t2] = result.value();
        if (t1 >= 0 && t2 > 0 && t2 < 1 && t1 < info.t) {
            info.t = t1;
            info.normal = normal[i];
        }
    }

    return info;
}

SweepHitInfo RaycastByCircle(const Vec2& start, const Vec2& dir,
                             const Circle& c) {
    SweepHitInfo info;
    if (IsPointInCircle(start, c)) {
        return info;
    }

    Vec2 pc = start - c.center;
    double A = dir.Dot(dir), B = 2 * pc.Dot(dir),
           C = pc.LengthSqrd() - c.radius * c.radius;
    double delta = B * B - 4 * A * C;
    if (delta < 0) {
        return info;
    }
    if (FLT_EQ(delta, 0)) {
        double toi = -B * 0.5;
        info.t = toi;
        return info;
    }
    info.t = (-B - std::sqrt(delta)) / (2.0 * A);
    info.normal = ((info.t * dir + start) - c.center).Normalize();

    return info;
}

SweepHitInfo RaycastByRoundAABB(const Vec2& start, const Vec2& dir,
                                float radius, const AABB& aabb) {
    const Circle circles[4] = {
        Circle{
               {aabb.center.x - aabb.halfSize.w, aabb.center.y - aabb.halfSize.h},
               radius},
        Circle{
               {aabb.center.x + aabb.halfSize.w, aabb.center.y - aabb.halfSize.h},
               radius},
        Circle{
               {aabb.center.x + aabb.halfSize.w, aabb.center.y + aabb.halfSize.h},
               radius},
        Circle{
               {aabb.center.x - aabb.halfSize.w, aabb.center.y + aabb.halfSize.h},
               radius},
    };
    const AABB aabbs[2] = {
        AABB{aabb.center, {aabb.halfSize.w, aabb.halfSize.h + radius}},
        AABB{aabb.center, {aabb.halfSize.w + radius, aabb.halfSize.h}},
    };

    std::array<SweepHitInfo, 16> hitInfoList;
    int count = 0;
    for (int i = 0; i < 4; i++) {
        auto result = RaycastByCircle(start, dir, circles[i]);
        if (result.t >= 0 && result.t <= 1) {
            hitInfoList[count++] = result;
        }
    }

    for (int i = 0; i < 2; i++) {
        auto result = RaycastByAABB(start, dir, aabbs[i]);
        if (result.t >= 0 && result.t <= 1) {
            hitInfoList[count++] = result;
        }
    }

    if (count == 0) {
        return {};
    }

    auto hit = std::min_element(
        std::begin(hitInfoList), std::begin(hitInfoList) + count,
        [](const SweepHitInfo& hit1, const SweepHitInfo& hit2) {
            return hit1.t < hit2.t;
        });

    return *hit;
}

bool IsPointInRect(const Vec2& p, const Rect& rect) {
    AABB aabb;
    aabb.halfSize = rect.size * 0.5;
    aabb.center = rect.position + aabb.halfSize;
    return IsPointInAABB(p, aabb);
}

bool IsPointInAABB(const Vec2& p, const AABB& aabb) {
    TL_RETURN_FALSE_IF_FALSE(aabb);

    return p.x > aabb.center.x - aabb.halfSize.w &&
           p.x < aabb.center.x + aabb.halfSize.w &&
           p.y > aabb.center.y - aabb.halfSize.h &&
           p.y < aabb.center.y + aabb.halfSize.h;
}

bool IsPointInCircle(const Vec2& p, const Circle& circle) {
    TL_RETURN_FALSE_IF_FALSE(circle);
    return (circle.center - p).LengthSqrd() < circle.radius * circle.radius;
}

bool IsAABBOverlap(const AABB& r1, const AABB& r2) {
    return !(r1.center.x + r1.halfSize.w <= r2.center.x - r2.halfSize.w ||
             r1.center.x - r1.halfSize.w >= r2.center.x + r2.halfSize.w ||
             r1.center.y - r1.halfSize.h >= r2.center.y + r2.halfSize.h ||
             r1.center.y + r1.halfSize.h <= r2.center.y - r2.halfSize.h);
}

bool IsCircleOverlap(const Circle& c1, const Circle& c2) {
    return (c1.center - c2.center).LengthSqrd() <
           (c1.radius + c2.radius) * (c1.radius + c2.radius);
}

bool IsCircleAABBOverlap(const Circle& c, const AABB& r) {
    Vec2 nearestPt = AABBNearestPoint(c.center, r);
    return (c.center - nearestPt).LengthSqrd() < c.radius * c.radius;
}

std::optional<std::tuple<float, float>> LinesIntersect(const Vec2& p1,
                                                       const Vec2& d1,
                                                       const Vec2& p2,
                                                       const Vec2& d2) {
    double delta = d2.Cross(d1);
    if (FLT_EQ(delta, 0)) {
        return std::nullopt;
    }

    Vec2 p1p2 = p2 - p1;
    float toi1 = p1p2.Cross(-d2) / delta;
    float toi2 = d1.Cross(p1p2) / delta;
    return std::make_tuple(toi1, toi2);
}

Vec2 AABBNearestPoint(const Vec2& p, const AABB& aabb) {
    Vec2 min = aabb.center - aabb.halfSize;
    Vec2 max = aabb.center + aabb.halfSize;

    Vec2 result;
    result.x = p.x < min.x ? min.x : (p.x > max.x ? max.x : p.x);
    result.y = p.y < min.y ? min.y : (p.y > max.y ? max.y : p.y);
    return result;
}

Vec2 CircleNearestPoint(const Vec2& p, const Circle& c) {
    return (p - c.center).Normalize() * c.radius + c.center;
}

Shape GetShapeRelateBy(const GameObject& go) {
    return GetShapeRelateBy(go.GetGlobalTransform(), go.physicActor);
}

Shape GetShapeRelateBy(const Transform& transform, const PhysicActor& actor) {
    Shape shape = actor.shape;
    
    Transform localTrans{shape.circle.center};
    Transform globalTrans = CalcTransformFromParent(transform, localTrans);
    shape.SetCenter(globalTrans.position);

    if (actor.shape.type == Shape::Type::Circle) {
        // NOTE: assume trans.scale.w == trans.scale.h
        shape.circle.radius = (actor.shape.circle.radius) *
                              std::abs(globalTrans.scale.w);

        // NOTE: hack way to generate bounding volume(AABB & Circle)
        shape.aabb.halfSize = Vec2{shape.circle.radius};
    } else if (actor.shape.type == Shape::Type::AABB) {
        shape.aabb.halfSize =
            (actor.shape.aabb.halfSize) * globalTrans.scale;
    } else {
        LOGW("unknown shape type in tile");
    }
    return shape;
}

}  // namespace tl
