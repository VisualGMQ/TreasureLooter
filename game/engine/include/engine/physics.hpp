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

class PhysicsShape;

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
    PhysicsShape *m_shape = nullptr;
};

struct OverlapResult {
    Entity m_dst_entity = null_entity;
    PhysicsShape *m_dst_shape = nullptr;
};

Rect RectUnion(const Rect &r1, const Rect &r2);

// nearest point
Vec2 NearestRectPoint(const Rect &, const Vec2 &);
Vec2 NearestCirclePoint(const Circle &, const Vec2 &);
Vec2 NearestCapsulePoint(const Vec2 &q, float r, const Vec2 &p1,
                         const Vec2 &p2);

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

enum class PhysicsStorageType {
    InChunk,
    Normal,
};

class PhysicsShape {
public:
    // for std::unique_ptr
    struct DeletorForProxy {
        void operator()(PhysicsShape *) const;
    };
    
    using Proxy = std::unique_ptr<PhysicsShape, DeletorForProxy>;
    
    enum class Type {
        Unknown = 0,
        Rect,
        Circle,
    };

    explicit PhysicsShape(Entity, PhysicsShapeDefinitionHandle,
                          PhysicsStorageType);
    explicit PhysicsShape(Entity, const PhysicsShapeDefinition &,
                          PhysicsStorageType);

    [[nodiscard]] const Rect *AsRect() const;

    [[nodiscard]] const Circle *AsCircle() const;

    [[nodiscard]] Type GetType() const;

    [[nodiscard]] PhysicsStorageType GetStorageType() const;

    [[nodiscard]] CollisionGroup GetCollisionLayer() const;
    void SetCollisionLayer(CollisionGroup);
    void SetCollisionMask(CollisionGroup);
    [[nodiscard]] CollisionGroup GetCollisionMask() const;

    [[nodiscard]] const Vec2 &GetPosition() const;

    void MoveTo(const Vec2 &p);

    void Move(const Vec2 &offset);

    void SetQueryEnable(bool enable);
    bool IsQueryEnabled() const;

    Entity GetOwner() const;

private:
    Entity m_owner = null_entity;
    Type m_type = Type::Unknown;
    bool m_enable_query = true;
    PhysicsStorageType m_storage_type;
    CollisionGroup m_collision_layer;
    CollisionGroup m_collision_mask;

    union {
        Rect m_rect{};
        Circle m_circle;
    };
};

class PhysicsScene {
public:
    using Chunk = MatStorage<std::vector<PhysicsShape *> >;

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
        // for std::unique_ptr
        struct DeletorForProxy {
            void operator()(TilemapCollision *tilemap_collision) const;
        };
        
        using Proxy = std::unique_ptr<TilemapCollision, DeletorForProxy>;

        Vec2 m_topleft;
        std::vector<Chunks> m_layers;
        std::vector<std::unique_ptr<PhysicsShape> > m_physics_shapes;

        explicit TilemapCollision(const Vec2 &topleft) : m_topleft(topleft) {}

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

    PhysicsShape *CreateShapeInChunk(Entity,
                                     TilemapCollision *tilemap_collision,
                                     PhysicsShapeDefinitionHandle);
    PhysicsShape *CreateShapeInChunk(Entity entity,
                                     TilemapCollision *tilemap_collision,
                                     const PhysicsShapeDefinition &);

    TilemapCollision *CreateTilemapCollision(const Vec2 &topleft);

    PhysicsShape *CreateShape(Entity, PhysicsShapeDefinitionHandle);

    void RemoveTilemapCollision(TilemapCollision *);

    void RemoveShape(PhysicsShape *);

    /*
     * @param dir is normalized vector
     */
    uint32_t Sweep(const PhysicsShape &, const Vec2 &dir, float dist,
                   SweepResult *out_result, size_t out_size);

    uint32_t Overlap(const PhysicsShape &, OverlapResult *out_result,
                     size_t out_size);

    [[nodiscard]] bool Overlap(const PhysicsShape &,
                               const PhysicsShape &) const;

    [[nodiscard]] bool IsEnableDebugDraw() const;

    void ToggleDebugDraw() { m_should_debug_draw = !m_should_debug_draw; }

    void RenderDebug() const;

private:
    std::vector<std::unique_ptr<TilemapCollision> > m_tilemap_collisions;

    std::vector<std::unique_ptr<PhysicsShape> > m_shapes;  // shapes in chunk
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

    [[nodiscard]] Rect computeShapeBoundingBox(const PhysicsShape &) const;
    [[nodiscard]] Rect computeShapeBoundingBox(
        const PhysicsShapeDefinition &) const;

    [[nodiscard]] std::optional<HitResult> sweepShape(const PhysicsShape &,
                                                      const PhysicsShape &actor,
                                                      const Vec2 &dir) const;

    [[nodiscard]] std::optional<HitResult> sweepGeometry(const Circle &,
                                                         const PhysicsShape &,
                                                         const Vec2 &dir) const;

    [[nodiscard]] std::optional<HitResult> sweepGeometry(const Rect &,
                                                         const PhysicsShape &,
                                                         const Vec2 &dir) const;

    [[nodiscard]] bool checkNeedQuery(const PhysicsShape &src,
                                      const PhysicsShape &target) const;

    void removeShapeInChunk(TilemapCollision *, uint32_t layer,
                            PhysicsShape *actor);
};
