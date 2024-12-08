#pragma once
#include "physics.hpp"
#include "tilemap.hpp"

namespace tl {
struct VectorSplitResult {
    Vec2 vertical;
    Vec2 horizontal;
};

VectorSplitResult SplitVector(const Vec2& dir, const Vec2& norm);

class PhysicsScene {
public:
    void MarkAsPhysics(GameObject*);
    void Update(TimeType delta);
    void ClearActors();
    size_t Raycast(const Vec2& start, const Vec2& dir, SweepHitInfo* outInfo,
                   size_t maxHitInfoCount);

    size_t Sweep(const Shape&, const Vec2& dir, SweepHitInfo*,
                 size_t maxHitInfoCount);

private:
    std::vector<MarkedActor> actors_;
    std::vector<SweepHitInfo> hitInfos_;

    void generateContacts();
    void sortContacts();
    void handleContacts();
    void handleNoContactMove();
    void moveAndSlide(SweepHitInfo& hit, float Threshould);
    bool quickCheckNeedSweep(const AABB& aabb1, const AABB& aabb2,
                             const Vec2& dir) const;

    SweepHitInfo sweep(const Shape& shape1, const Shape& shape2,
                       const Vec2& dir);
    void handleTrigger();
    bool checkOverlap(const PhysicActor& tirgger, const PhysicActor& overlap) const;
};
}
