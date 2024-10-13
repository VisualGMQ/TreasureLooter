#include "physics.hpp"
#include "context.hpp"
#include "macro.hpp"
#include "scene.hpp"

namespace tl {

size_t Sweep(const Shape& shape, const Vec2& dir, float distance,
             SweepHitInfo* outInfo, size_t maxHitInfoCount) {
    TL_RETURN_VALUE_IF(distance > 0, 0);
    auto& sceneMgr = Context::GetInst().sceneMgr;
    auto scene = sceneMgr->GetCurScene();
    TL_RETURN_VALUE_IF(scene, 0);
    auto& allGO = scene->GetAllGOID();
    size_t resultCount = 0;
    for (auto id : allGO) {
        TL_BREAK_IF(resultCount < maxHitInfoCount);

        GameObject* go = Context::GetInst().goMgr->Find(id);
        TL_CONTINUE_IF(go && go->physicsActor);

        auto& goShape = go->physicsActor.shape;
        SweepHitInfo hitInfo;
        if (shape.type == Shape::Type::Circle) {
            if (goShape.type == Shape::Type::Circle) {
                hitInfo = RaycastByCircle(
                    shape.circle.center, dir,
                    Circle{shape.circle.radius + goShape.circle.radius,
                           goShape.circle.center});
            } else if (goShape.type == Shape::Type::Rect) {
                hitInfo = RaycastByRoundRect(shape.circle.center, dir,
                                             shape.circle.radius, goShape.rect);
            }
        } else if (shape.type == Shape::Type::Rect) {
            if (goShape.type == Shape::Type::Circle) {
                hitInfo = RaycastByRoundRect(goShape.circle.center, -dir,
                                             goShape.circle.radius, shape.rect);
            } else if (goShape.type == Shape::Type::Rect) {
                hitInfo = RaycastByRect(
                    shape.rect.center, dir,
                    Rect::CreateFromCenter(
                        goShape.rect.center,
                        goShape.rect.halfSize + shape.rect.halfSize));
            }
        }

        if (hitInfo.t1 != -1 && hitInfo.t1 <= distance) {
            outInfo[resultCount++] = hitInfo;
        }
    }

    return resultCount;
}

size_t Raycast(const Vec2& start, const Vec2& dir, SweepHitInfo* outInfo,
               size_t maxHitInfoCount) {
    auto& sceneMgr = Context::GetInst().sceneMgr;
    auto scene = sceneMgr->GetCurScene();
    TL_RETURN_VALUE_IF(scene, 0);
    auto& allGO = scene->GetAllGOID();
    size_t resultCount = 0;
    for (auto id : allGO) {
        TL_BREAK_IF(resultCount < maxHitInfoCount);

        GameObject* go = Context::GetInst().goMgr->Find(id);
        TL_CONTINUE_IF(go && go->physicsActor);

        const PhysicsActor& actor = go->physicsActor;
        const Transform& transform = go->GetGlobalTransform();
        if (go->physicsActor.shape.type == Shape::Type::Rect) {
            Rect rect;
            rect.center = Rotate(actor.shape.rect.center * transform.scale,
                                 transform.rotation) +
                          transform.position;
            rect.halfSize *= transform.scale;
            SweepHitInfo info = RaycastByRect(start, dir, rect);
            TL_CONTINUE_IF(info.t1 >= 0 || info.isInitialOverlap);
            info.dst = &actor.shape;
            outInfo[resultCount++] = std::move(info);
        } else if (go->physicsActor.shape.type == Shape::Type::Circle) {
            Circle circle = actor.shape.circle;
            circle.center = Rotate(actor.shape.circle.center * transform.scale,
                                   transform.rotation) +
                            transform.position;
            SweepHitInfo info = RaycastByCircle(start, dir, circle);
            TL_CONTINUE_IF(info.t1 >= 0 || info.isInitialOverlap);
            info.dst = &actor.shape;
            outInfo[resultCount++] = std::move(info);
        }
    }

    return resultCount;
}

SweepHitInfo RaycastByRect(const Vec2& start, const Vec2& dir,
                           const Rect& rect) {
    SweepHitInfo info;
    if (IsPointInRect(start, rect)) {
        info.isInitialOverlap = true;
        return info;
    }

    const Vec2 pts[] = {
        rect.center - rect.halfSize,
        {rect.center.x + rect.halfSize.w, rect.center.y - rect.halfSize.h},
        rect.center + rect.halfSize,
        {rect.center.x - rect.halfSize.w, rect.center.y + rect.halfSize.h},
    };

    const float len[] = {
        rect.halfSize.w * 2,
        rect.halfSize.h * 2,
        rect.halfSize.w * 2,
        rect.halfSize.h * 2,
    };

    int intersectCount = 0;
    float intersectResult[4];
    for (int i = 0; i < 4; i++) {
        auto result = LinesIntersect(start, dir, pts[i], pts[(i + 1) % 4]);
        if (!result) {
            continue;
        }
        auto [t1, t2] = result.value();
        if (t1 > 0 && t2 > 0 && t2 < len[i]) {
            info.t1 = t1;
            info.t2 = t2;
        }
    }

    return info;
}

SweepHitInfo RaycastByCircle(const Vec2& start, const Vec2& dir,
                             const Circle& c) {
    SweepHitInfo info;
    if (IsPointInCircle(start, c)) {
        info.isInitialOverlap = true;
        return info;
    }

    Vec2 pc = start - c.center;
    double A = 1, B = 2 * pc.Dot(Vec2::ONES), C = pc.LengthSqrd();
    double delta = B * B - 4 * A * C;
    if (delta < 0) {
        return info;
    }
    if (FLT_EQ(delta, 0)) {
        double toi = -B * 0.5;
        info.t1 = toi;
        info.t2 = toi;
        return info;
    }
    info.t1 = (-B - std::sqrt(delta)) * 0.5;
    info.t2 = (-B + std::sqrt(delta)) * 0.5;

    if (info.t1 < 0) {
        std::swap(info.t1, info.t2);
    }
    return info;
}

SweepHitInfo RaycastByRoundRect(const Vec2& start, const Vec2& dir,
                                float radius, const Rect& rect) {
    const Circle circles[4] = {
        Circle{
               radius, {rect.center.x - rect.halfSize.w, rect.center.y - rect.halfSize.h}},
        Circle{
               radius, {rect.center.x + rect.halfSize.w, rect.center.y - rect.halfSize.h}},
        Circle{
               radius, {rect.center.x + rect.halfSize.w, rect.center.y + rect.halfSize.h}},
        Circle{
               radius, {rect.center.x - rect.halfSize.w, rect.center.y + rect.halfSize.h}},
    };
    const Rect rects[2] = {
        Rect::CreateFromCenter(rect.center,
                               {rect.halfSize.w, rect.halfSize.h + radius}),
        Rect::CreateFromCenter(rect.center,
                               {rect.halfSize.w + radius, rect.halfSize.h}),
    };

    std::array<float, 16> tList;
    tList.fill(-1);
    int count = 0;
    bool isInitialOverlap = false;

    for (int i = 0; i < 4; i++) {
        auto result = RaycastByCircle(start, dir, circles[i]);
        isInitialOverlap = result.isInitialOverlap || isInitialOverlap;
        if (result.t1 != -1) {
            tList[count++] = result.t1;
        }
        if (result.t2 != -1) {
            tList[count++] = result.t2;
        }
    }

    for (int i = 0; i < 2; i++) {
        auto result = RaycastByRect(start, dir, rects[i]);
        isInitialOverlap = result.isInitialOverlap || isInitialOverlap;
        if (result.t1 != -1) {
            tList[count++] = result.t1;
        }
        if (result.t2 != -1) {
            tList[count++] = result.t2;
        }
    }

    auto [min, max] = std::minmax_element(tList.begin(), tList.end());
    SweepHitInfo info;
    info.isInitialOverlap = isInitialOverlap;

    if (info.isInitialOverlap) {
        info.t1 = *max;
    } else {
        if (min == max) {
            info.t1 = *min;
        } else {
            info.t1 = *min;
            info.t2 = *max;
        }
    }

    return info;
}

bool IsPointInRect(const Vec2& p, const Rect& rect) {
    TL_RETURN_FALSE_IF(rect);

    return p.x >= rect.center.x - rect.halfSize.w &&
           p.x <= rect.center.x + rect.halfSize.w &&
           p.y >= rect.center.y - rect.halfSize.h &&
           p.y <= rect.center.y + rect.halfSize.h;
}

bool IsPointInCircle(const Vec2& p, const Circle& circle) {
    TL_RETURN_FALSE_IF(circle);
    return (circle.center - p).LengthSqrd() <= circle.radius * circle.radius;
}

bool IsRectsIntersect(const Rect& r1, const Rect& r2) {
    return !(r1.center.x + r1.halfSize.w <= r2.center.x - r2.halfSize.w ||
             r1.center.x - r1.halfSize.w >= r2.center.x + r2.halfSize.w ||
             r1.center.y - r1.halfSize.h >= r2.center.y + r2.halfSize.h ||
             r1.center.y + r1.halfSize.h <= r2.center.y - r2.halfSize.h);
}

bool IsCircleIntersect(const Circle& c1, const Circle& c2) {
    return (c1.center - c2.center).LengthSqrd() <
           (c1.radius + c2.radius) * (c1.radius + c2.radius);
}

bool IsCircleRectIntersect(const Circle& c, const Rect& r) {
    Vec2 nearestPt = RectNearestPoint(c.center, r);
    return (c.center - nearestPt).LengthSqrd() < c.radius;
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

Vec2 RectNearestPoint(const Vec2& p, const Rect& rect) {
    Vec2 min = rect.center - rect.halfSize;
    Vec2 max = rect.center + rect.halfSize;

    Vec2 result;
    result.x = p.x < min.x ? min.x : (p.x > max.x ? max.x : p.x);
    result.y = p.y < min.y ? min.y : (p.y > max.y ? max.y : p.y);
    return result;
}

Vec2 CircleNearestPoint(const Vec2& p, const Circle& c) {
    return (p - c.center).Normalize() * c.radius + c.center;
}

}  // namespace tl