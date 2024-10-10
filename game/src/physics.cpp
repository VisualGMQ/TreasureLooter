#include "physics.hpp"
#include "macro.hpp"
#include "context.hpp"
#include "scene.hpp"

namespace tl {

size_t Sweep(const Shape& shape, const Vec2& dir, float distance) {
    TL_RETURN_VALUE_IF(distance > 0, 0);
    // TODO:
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

}  // namespace tl