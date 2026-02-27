#pragma once
#include "engine/collision_group.hpp"
#include "engine/entity.hpp"
#include "engine/flag.hpp"
#include "engine/math.hpp"
#include "schema/common.hpp"
#include "schema/physics_schema.hpp"

#include <memory>
#include <optional>
#include <vector>

class PhysicsActor;

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
    bool m_is_initial_overlap = false;

    HitResult() = default;

    HitResult(float t, Flags<HitType> f, Vec2 n, bool is_initial_overlap);

    virtual ~HitResult() = default;
};

struct SweepResult : HitResult {
    Entity m_entity = null_entity;
    PhysicsActor *m_actor = nullptr;
};

struct OverlapResult {
    Entity m_dst_entity = null_entity;
    PhysicsActor *m_dst_actor = nullptr;
};

Rect RectUnion(const Rect& r1, const Rect& r2);

// nearest point
Vec2 NearestRectPoint(const Rect &, const Vec2 &);
Vec2 NearestCirclePoint(const Circle &, const Vec2 &);
Vec2 NearestCapsulePoint(const Vec2& q, float r, const Vec2& p1, const Vec2& p2);

// overlap
bool IsPointInRect(const Vec2 &, const Rect &);

bool IsRectsIntersect(const Rect &, const Rect &);

bool IsPointInCircle(const Vec2 &, const Circle &);

bool IsCirclesIntersect(const Circle &, const Circle &);

bool IsCircleRectIntersect(const Circle &, const Rect &);

// raycast
std::optional<std::pair<float, float> > RayIntersect(const Vec2 &p1,
                                                     const Vec2 &dir1,
                                                     const Vec2 &p2,
                                                     const Vec2 &dir2);

std::optional<HitResult> RaycastRect(const Vec2 &p, const Vec2 &dir,
                                     const Rect &);

std::optional<HitResult> RaycastCircle(const Vec2 &p, const Vec2 &dir,
                                       const Circle &);

// sweep
std::optional<HitResult> SweepRects(const Rect &r1, const Rect &r2,
                                    const Vec2 &dir);

std::optional<HitResult> SweepCircles(const Circle &c1, const Circle &c2,
                                      const Vec2 &dir);

std::optional<HitResult> SweepCircleRect(const Circle &c, const Rect &r,
                                         const Vec2 &dir);

enum class PhysicsShapeType {
    Unknown = 0,
    Rect,
    Circle,
};

class PhysicsShape {
public:
    explicit PhysicsShape(const Rect &r);

    explicit PhysicsShape(const Circle &c);

    [[nodiscard]] const Rect *AsRect() const;

    [[nodiscard]] const Circle *AsCircle() const;

    [[nodiscard]] PhysicsShapeType GetType() const;

    [[nodiscard]] const Vec2 &GetPosition() const;

    void MoveTo(const Vec2 &p);

    void Move(const Vec2 &offset);

private:
    PhysicsShapeType m_type = PhysicsShapeType::Unknown;

    union {
        Rect m_rect{};
        Circle m_circle;
    };
};

class PhysicsActor {
public:
    enum class StorageType {
        InChunk,
        Normal,
    };

    explicit PhysicsActor(Entity entity, const Rect &r, StorageType storage);

    explicit PhysicsActor(Entity entity, const Circle &c, StorageType storage);

    explicit PhysicsActor(Entity entity, const PhysicsShape &,
                          StorageType storage);

    [[nodiscard]] const PhysicsShape &GetShape() const;

    [[nodiscard]] StorageType GetStorageType() const;

    [[nodiscard]] const Vec2 &GetPosition() const;

    void SetCollisionLayer(CollisionGroup collision_group);

    [[nodiscard]] auto GetCollisionLayer() const { return m_collision_layer; }

    void SetCollisionMask(CollisionGroup collision_group);

    [[nodiscard]] auto GetCollisionMask() const { return m_collision_mask; }

    [[nodiscard]] Entity GetEntity() const { return m_owner; }

    void MoveTo(const Vec2 &position);

    void Move(const Vec2 &offset);

private:
    PhysicsShape m_shape;
    Entity m_owner = null_entity;
    StorageType m_storage_type;
    CollisionGroup m_collision_layer;
    CollisionGroup m_collision_mask;
};

class PhysicsScene {
public:
    using Chunk = MatStorage<std::vector<PhysicsActor *> >;

    struct Chunks {
        Vec2UI m_chunk_size{};
        Vec2UI m_tile_size{};
        MatStorage<Chunk> m_chunks;

        Chunks() = default;

        Chunks(const Chunks &) = delete;

        Chunks &operator=(const Chunks &) = delete;

        Chunks(Chunks &&) = default;

        Chunks &operator=(Chunks &&) = default;

        void getOverlapChunkRange(const Rect &bounding_box,
                                  Range2D<int> &out_chunk_range,
                                  Range2D<int> &out_tile_range);

        void getTileRangeInCurrentChunk(const Range2D<int> &chunk_range,
                                        const Range2D<int> &tile_range, int x,
                                        int y, Range2D<int> &out_tile_range);
    };

    struct TilemapCollision {
        Vec2 m_topleft;
        std::vector<Chunks> m_layers;
        std::vector<std::unique_ptr<PhysicsActor> > m_actors;

        explicit TilemapCollision(const Vec2 &topleft) : m_topleft(topleft) {
        }

        TilemapCollision(const TilemapCollision &) = delete;

        TilemapCollision &operator=(const TilemapCollision &) = delete;

        TilemapCollision(TilemapCollision &&) = default;

        TilemapCollision &operator=(TilemapCollision &&) = default;

        Chunks &CreateLayer(const Vec2UI &tile_size, const Vec2UI &chunk_size) {
            Chunks layer;
            layer.m_chunk_size = chunk_size;
            layer.m_tile_size = tile_size;
            return m_layers.emplace_back(std::move(layer));
        }
    };

    PhysicsScene();

    PhysicsActor *CreateActorInChunk(Entity,
                                     TilemapCollision *tilemap_collision,
                                     uint32_t layer, const PhysicsShape &);

    TilemapCollision *CreateTilemapCollision(const Vec2 &topleft);

    PhysicsActor *CreateActor(Entity, PhysicsActorInfoHandle info);

    PhysicsActor *CreateActor(Entity, const Circle &);

    PhysicsActor *CreateActor(Entity, const Rect &);

    void RemoveTilemapCollision(TilemapCollision *);

    void RemoveActor(PhysicsActor *);

    void RemoveActorInChunk(TilemapCollision *, uint32_t layer,
                            PhysicsActor *actor);

    /*
     * @param dir is normalized vector
     */
    uint32_t Sweep(const PhysicsShape &, CollisionGroup mask, const Vec2 &dir,
                   float dist, SweepResult *out_result, size_t out_size);

    uint32_t Sweep(const PhysicsActor &, const Vec2 &dir, float dist,
                   SweepResult *out_result, size_t out_size);

    uint32_t Overlap(const PhysicsShape &, CollisionGroup mask,
                     OverlapResult *out_result, size_t out_size);

    uint32_t Overlap(const PhysicsActor &, OverlapResult *out_result,
                     size_t out_size);

    [[nodiscard]] bool Overlap(const PhysicsActor &, const PhysicsActor &) const;

    [[nodiscard]] bool Overlap(const PhysicsShape &, const PhysicsShape &) const;

    [[nodiscard]] bool IsEnableDebugDraw() const;

    void ToggleDebugDraw() { m_should_debug_draw = !m_should_debug_draw; }

    void RenderDebug() const;

private:
    std::vector<std::unique_ptr<TilemapCollision> > m_tilemap_collisions;

    std::vector<std::unique_ptr<PhysicsActor> > m_actors; // actors not in chunk
    std::vector<SweepResult> m_cached_sweep_results;
    std::vector<OverlapResult> m_cached_overlaps_results;
    bool m_should_debug_draw = false;

    [[nodiscard]] Rect computeSweepBoundingBox(const Rect &, const Vec2 &dir,
                                               float dist) const;

    [[nodiscard]] Rect computeSweepBoundingBox(const Circle &, const Vec2 &dir,
                                               float dist) const;

    [[nodiscard]] Rect computeSweepBoundingBox(const PhysicsShape &,
                                               const Vec2 &dir,
                                               float dist) const;

    [[nodiscard]] Rect computeSweepBoundingBox(const PhysicsActor &,
                                               const Vec2 &dir,
                                               float dist) const;

    [[nodiscard]] Rect computeActorBoundingBox(const PhysicsActor &) const;

    [[nodiscard]] Rect computeShapeBoundingBox(const PhysicsShape &) const;

    [[nodiscard]] std::optional<HitResult> sweepActor(const PhysicsActor &,
                                                      const PhysicsActor &,
                                                      const Vec2 &dir) const;

    [[nodiscard]] std::optional<HitResult> sweepShape(const PhysicsShape &,
                                                      const PhysicsActor &actor,
                                                      const Vec2 &dir) const;

    [[nodiscard]] std::optional<HitResult> sweepGeometry(
        const Circle &, const PhysicsActor &actor, const Vec2 &dir) const;

    [[nodiscard]] std::optional<HitResult> sweepGeometry(
        const Rect &, const PhysicsActor &actor, const Vec2 &dir) const;

    [[nodiscard]] bool checkNeedQuery(const PhysicsShape &a,
                                      CollisionGroup mask,
                                      const PhysicsActor &b) const;
};
