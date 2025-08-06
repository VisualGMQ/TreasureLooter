#pragma once
#include "flag.hpp"
#include "math.hpp"
#include "schema/common.hpp"

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

    explicit PhysicsActor(const Rect& r, StorageType storage);

    explicit PhysicsActor(const Circle& c, StorageType storage);

    const Rect* AsRect() const;

    const Circle* AsCircle() const;

    ShapeType GetShapeType() const;

    StorageType GetStorageType() const;

    const Vec2& GetPosition() const;

private:
    union {
        Rect m_rect{};
        Circle m_circle;
    };

    ShapeType m_type;
    StorageType m_storage_type;
};

class PhysicsScene {
public:
    using Chunk = MatStorage<std::vector<std::unique_ptr<PhysicsActor>>>;

    PhysicsScene();

    PhysicsActor* CreateActorInChunk(size_t chunk_x, size_t chunk_y, size_t x,
                                     size_t y, const Rect&);
    PhysicsActor* CreateActorInChunk(size_t chunk_x, size_t chunk_y, size_t x,
                                     size_t y, const Circle&);
    PhysicsActor* CreateActorInChunk(const Vec2& center, const Rect&);
    PhysicsActor* CreateActorInChunk(const Vec2& center, const Circle&);
    PhysicsActor* CreateActor(const Circle&);
    PhysicsActor* CreateActor(const Rect&);
    void RemoveActor(PhysicsActor*);

    /*
     * @param dir normalized vector
     */
    bool Sweep(const PhysicsActor&, const Vec2& dir, float dist,
                     HitResult* out_result, size_t out_size);

    bool IsEnableDebugDraw() const { return m_should_debug_draw; }

    void ToggleDebugDraw() { m_should_debug_draw = !m_should_debug_draw; }

    void RenderDebug() const;

private:
    using Chunks = MatStorage<std::unique_ptr<Chunk>>;

    Chunks m_chunks;
    std::vector<std::unique_ptr<PhysicsActor>> m_actors;  // actors not in chunk
    std::vector<HitResult> m_cached_hits;
    bool m_should_debug_draw = false;

    Chunk& ensureChunk(size_t x, size_t y);
    PhysicsActor* setActor2Chunk(size_t chunk_x, size_t chunk_y, size_t x,
                                 size_t y, std::unique_ptr<PhysicsActor>&&);

    [[nodiscard]] Rect computeSweepBoundingBox(const Rect&, const Vec2& dir,
                                               float dist) const;
    [[nodiscard]] Rect computeSweepBoundingBox(const Circle&, const Vec2& dir,
                                               float dist) const;
    [[nodiscard]] Rect computeSweepBoundingBox(const PhysicsActor&, const Vec2& dir,
                                               float dist) const;

    std::vector<std::unique_ptr<PhysicsActor>>* getActorStoreInChunk(
        const Vec2& actor_center, bool ensure);
    [[nodiscard]] Rect computeActorBoundingBox(const PhysicsActor&) const;

    std::optional<HitResult> sweepActor(const PhysicsActor&,
                                        const PhysicsActor&, const Vec2& dir) const;
};