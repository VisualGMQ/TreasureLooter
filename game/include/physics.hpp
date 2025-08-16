#pragma once
#include "collision_group.hpp"
#include "entity.hpp"
#include "flag.hpp"
#include "math.hpp"
#include "schema/common.hpp"
#include "schema/physics_schema.hpp"

#include <memory>
#include <optional>
#include <vector>

enum class HitType {
    None = 0,

    Left = 0x01,
    Right = 0x02,
    Top = 0x04,
    Bottom = 0x08,

    LeftTopCorner = Left | Top,
    RightTopCorner = Right | Top,
    LeftBottomCorner = Left | Bottom,
    RightBottomCorner = Right | Bottom,
};

struct HitResult {
    float m_t{};
    Flags<HitType> m_flags = HitType::None;
    Vec2 m_normal;
};

class PhysicsActor;

struct OverlapResult {
    Entity m_src_entity = null_entity;
    Entity m_dst_entity = null_entity;
    const PhysicsActor* m_src_actor = nullptr;
    PhysicsActor* m_dst_actor = nullptr;
};

// nearest point
Vec2 NearestRectPoint(const Rect&, const Vec2&);
Vec2 NearestCirclePoint(const Circle&, const Vec2&);

// overlap
bool IsPointInRect(const Vec2&, const Rect&);
bool IsRectsIntersect(const Rect&, const Rect&);
bool IsPointInCircle(const Vec2&, const Circle&);
bool IsCirclesIntersect(const Circle&, const Circle&);
bool IsCircleRectIntersect(const Circle&, const Rect&);

// raycast
std::optional<std::pair<float, float>> RayIntersect(const Vec2& p1,
                                                    const Vec2& dir1,
                                                    const Vec2& p2,
                                                    const Vec2& dir2);
std::optional<HitResult> RaycastRect(const Vec2& p, const Vec2& dir,
                                     const Rect&);
std::optional<float> RaycastCircle(const Vec2& p, const Vec2& dir,
                                   const Circle&);

// sweep
std::optional<HitResult> SweepRects(const Rect& r1, const Rect& r2,
                                    const Vec2& dir);
std::optional<float> SweepCircles(const Circle& c1, const Circle& c2,
                                  const Vec2& dir);
std::optional<HitResult> SweepCircleRect(const Circle& c, const Rect& r,
                                         const Vec2& dir);

class PhysicsActor {
public:
    enum class StorageType {
        InChunk,
        Normal,
    };

    enum class ShapeType {
        Unknown = 0,
        Rect,
        Circle,
    };

    explicit PhysicsActor(Entity entity, const Rect& r, StorageType storage);

    explicit PhysicsActor(Entity entity, const Circle& c, StorageType storage);

    const Rect* AsRect() const;

    const Circle* AsCircle() const;

    ShapeType GetShapeType() const;

    StorageType GetStorageType() const;

    const Vec2& GetPosition() const;

    void SetCollisionLayer(CollisionGroup collision_group);

    auto GetCollisionLayer() const { return m_collision_layer; }

    void SetCollisionMask(CollisionGroup collision_group);

    auto GetCollisionMask() const { return m_collision_mask; }

    Entity GetEntity() const { return m_owner; }

    void MoveTo(const Vec2& position);

private:
    union {
        Rect m_rect{};
        Circle m_circle;
    };

    Entity m_owner = null_entity;
    ShapeType m_type;
    StorageType m_storage_type;
    CollisionGroup m_collision_layer;
    CollisionGroup m_collision_mask;
};

class PhysicsScene {
public:
    using Chunk = MatStorage<std::vector<std::unique_ptr<PhysicsActor>>>;

    PhysicsScene();

    PhysicsActor* CreateActorInChunk(Entity, size_t chunk_x, size_t chunk_y,
                                     size_t x, size_t y, const Rect&);
    PhysicsActor* CreateActorInChunk(Entity, size_t chunk_x, size_t chunk_y,
                                     size_t x, size_t y, const Circle&);
    PhysicsActor* CreateActorInChunk(Entity, const Vec2& center, const Rect&);
    PhysicsActor* CreateActorInChunk(Entity, const Vec2& center, const Circle&);
    PhysicsActor* CreateActor(Entity, PhysicsActorInfoHandle info);
    PhysicsActor* CreateActor(Entity, const Circle&);
    PhysicsActor* CreateActor(Entity, const Rect&);
    void RemoveActor(PhysicsActor*);

    /*
     * @param dir is normalized vector
     */
    bool Sweep(const PhysicsActor&, const Vec2& dir, float dist,
               HitResult* out_result, size_t out_size);

    uint32_t Overlap(const PhysicsActor&, OverlapResult* out_result,
                     size_t out_size);
    [[nodiscard]] bool Overlap(const PhysicsActor&, const PhysicsActor&) const;

    [[nodiscard]] bool IsEnableDebugDraw() const;

    void ToggleDebugDraw() { m_should_debug_draw = !m_should_debug_draw; }

    void RenderDebug() const;

private:
    using Chunks = MatStorage<std::unique_ptr<Chunk>>;

    Chunks m_chunks;
    std::vector<std::unique_ptr<PhysicsActor>> m_actors;  // actors not in chunk
    std::vector<HitResult> m_cached_hits;
    std::vector<OverlapResult> m_cached_overlaps;
    bool m_should_debug_draw = false;

    Chunk& ensureChunk(size_t x, size_t y);
    PhysicsActor* setActor2Chunk(size_t chunk_x, size_t chunk_y, size_t x,
                                 size_t y, std::unique_ptr<PhysicsActor>&&);

    [[nodiscard]] Rect computeSweepBoundingBox(const Rect&, const Vec2& dir,
                                               float dist) const;
    [[nodiscard]] Rect computeSweepBoundingBox(const Circle&, const Vec2& dir,
                                               float dist) const;
    [[nodiscard]] Rect computeSweepBoundingBox(const PhysicsActor&,
                                               const Vec2& dir,
                                               float dist) const;

    std::vector<std::unique_ptr<PhysicsActor>>* getActorStoreInChunk(
        const Vec2& actor_center, bool ensure);
    [[nodiscard]] Rect computeActorBoundingBox(const PhysicsActor&) const;

    [[nodiscard]] std::optional<HitResult> sweepActor(const PhysicsActor&,
                                                      const PhysicsActor&,
                                                      const Vec2& dir) const;

    [[nodiscard]] bool checkNeedQuery(const PhysicsActor&,
                                      const PhysicsActor&) const;

    void getOverlapChunkRange(const Rect& bounding_box,
                              Range2D<int>& out_chunk_range,
                              Range2D<int>& out_tile_range);
    void getTileRangeInCurrentChunk(Range2D<int>& chunk_range,
                                    Range2D<int>& tile_range, int x, int y,
                                    Range2D<int>& out_tile_range);
};
