#pragma once
#include "math.hpp"
#include "flag.hpp"
#include "schema/common.hpp"

#include <optional>
#include <vector>

// overlap
bool IsPointInRect(const Vec2&, const Rect&);
bool IsRectsIntersect(const Rect&, const Rect&);
bool IsPointInCircle(const Vec2&, const Circle&);
bool IsCirclesIntersect(const Circle&, const Circle&);
bool IsCircleRectIntersect(const Circle&, const Rect&);

enum class HitType {
    None = 0,

    Left = 0x01,
    Right = 0x02,
    Top = 0x04,
    Bottom = 0x08,

    LeftTopCorner = Left | Top,
    RightTopCorner = Right| Top,
    LeftBottomCorner = Left | Bottom,
    RightBottomCorner = Right| Bottom,
};

struct HitResult {
    float m_t{};
    Flags<HitType> m_flags = HitType::None;
    Vec2 m_normal;
};

// raycast
std::optional<std::pair<float, float>> RayIntersect(const Vec2& p1, const Vec2& dir1, const Vec2& p2, const Vec2& dir2);
std::optional<HitResult> RaycastRect(const Vec2& p, const Vec2& dir, const Rect&);
std::optional<float> RaycastCircle(const Vec2& p, const Vec2& dir, const Circle&);

// sweep
std::optional<HitResult> SweepRects(const Rect& r1, const Rect& r2, const Vec2& dir);
std::optional<float> SweepCircles(const Circle& c1, const Circle& c2, const Vec2& dir);
std::optional<HitResult> SweepCircleRect(const Circle& c, const Rect& r, const Vec2& dir);

class PhysicsScene {
public:
    PhysicsScene();

    void AddRect(const Rect&);
    void AddCircle(const Circle&);

    /*
     * @param dir normalized vector
     */
    bool SweepByRect(const Rect&, const Vec2& dir, float dist, HitResult* out_result, size_t out_size);

    /*
     * @param dir normalized vector
     */
    bool SweepByCircle(const Circle&, const Vec2& dir, float dist, HitResult* out_result, size_t out_size);

    bool IsEnableDebugDraw() const { return m_should_debug_draw; }
    void ToggleDebugDraw() { m_should_debug_draw = !m_should_debug_draw; }
    void RenderDebug() const;

private:
    std::vector<Rect> m_rects;
    std::vector<Circle> m_circle;

    std::vector<HitResult> m_cached_hits;

    bool m_should_debug_draw = false;
};