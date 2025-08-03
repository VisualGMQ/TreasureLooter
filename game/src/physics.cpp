#include "physics.hpp"

#include "context.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

bool IsPointInRect(const Vec2& p, const Rect& r) {
    return p.x >= r.m_center.x - r.m_half_size.x &&
           p.x <= r.m_center.x + r.m_half_size.x &&
           p.y >= r.m_center.y - r.m_half_size.y &&
           p.y <= r.m_center.y + r.m_half_size.y;
}

bool IsRectsIntersect(const Rect& r1, const Rect& r2) {
    Rect r = r1;
    r.m_half_size += r2.m_half_size;

    return IsPointInRect(r2.m_center, r);
}

bool IsPointInCircle(const Vec2& p, const Circle& c) {
    return (p - c.m_center).LengthSquared() <= c.m_radius * c.m_radius;
}

bool IsCirclesIntersect(const Circle& c1, const Circle& c2) {
    auto radius_sum = c1.m_radius + c2.m_radius;
    return (c1.m_center - c2.m_center).LengthSquared() <=
           radius_sum * radius_sum;
}

bool IsCircleRectIntersect(const Circle& c, const Rect& r) {
    Rect r1 = r, r2 = r;
    r1.m_half_size.x += c.m_radius;
    r2.m_half_size.y += c.m_radius;

    Circle c1 = c, c2 = c, c3 = c, c4 = c;
    c1.m_center = r.m_center - r.m_half_size;
    c2.m_center = r.m_center + r.m_half_size;
    c3.m_center = {r.m_center.x + r.m_half_size.x,
                   r.m_center.y - r.m_half_size.y};
    c4.m_center = {r.m_center.x - r.m_half_size.x,
                   r.m_center.y + r.m_half_size.y};

    return IsPointInRect(c.m_center, r1) || IsPointInRect(c.m_center, r2) ||
           IsPointInCircle(c.m_center, c1) || IsPointInCircle(c.m_center, c2) ||
           IsPointInCircle(c.m_center, c3) || IsPointInCircle(c.m_center, c4);
}

std::optional<std::pair<float, float>> RayIntersect(const Vec2& p1,
                                                    const Vec2& dir1,
                                                    const Vec2& p2,
                                                    const Vec2& dir2) {
    float delta = dir1.Cross(-dir2);

    if (std::abs(delta) <= std::numeric_limits<float>::epsilon()) {
        return std::nullopt;
    }

    Vec2 p_diff = p2 - p1;
    float t1 = p_diff.Cross(-dir2) / delta;
    float t2 = dir1.Cross(p_diff) / delta;

    if (t1 >= 0 && t2 >= 0) return std::make_pair(t1, t2);
    return std::nullopt;
}

std::optional<HitResult> RaycastRect(const Vec2& p, const Vec2& dir,
                                     const Rect& r) {
    Vec2 top_left = r.m_center - r.m_half_size;
    Vec2 bottom_right = r.m_center + r.m_half_size;
    auto t1 =
        RayIntersect(p, dir, top_left, Vec2::X_UNIT * r.m_half_size.x * 2);
    auto t2 =
        RayIntersect(p, dir, top_left, Vec2::Y_UNIT * r.m_half_size.y * 2);
    auto t3 =
        RayIntersect(p, dir, bottom_right, -Vec2::X_UNIT * r.m_half_size.x * 2);
    auto t4 =
        RayIntersect(p, dir, bottom_right, -Vec2::Y_UNIT * r.m_half_size.y * 2);

    if (t1 && (t1->second < 0 || t1->second > 1)) {
        t1 = std::nullopt;
    }
    if (t2 && (t2->second < 0 || t2->second > 1)) {
        t2 = std::nullopt;
    }
    if (t3 && (t3->second < 0 || t3->second > 1)) {
        t3 = std::nullopt;
    }
    if (t4 && (t4->second < 0 || t4->second > 1)) {
        t4 = std::nullopt;
    }

    if (!t1 && !t2 && !t3 && !t4) {
        return std::nullopt;
    }

    std::array<float, 4> t_list = {
        t1 ? t1->first : std::numeric_limits<float>::infinity(),
        t2 ? t2->first : std::numeric_limits<float>::infinity(),
        t3 ? t3->first : std::numeric_limits<float>::infinity(),
        t4 ? t4->first : std::numeric_limits<float>::infinity(),
    };

    constexpr std::array<HitType, 4> hit_types = {
        HitType::Top,
        HitType::Left,
        HitType::Bottom,
        HitType::Right,
    };

    static const std::array<Vec2, 4> hit_normals = {
        -Vec2::Y_UNIT,
        -Vec2::X_UNIT,
        Vec2::Y_UNIT,
        Vec2::X_UNIT,
    };

    auto it = std::min_element(t_list.begin(), t_list.end());
    size_t idx = it - t_list.begin();
    return HitResult{*it, hit_types[idx], hit_normals[idx]};
}

std::optional<float> RaycastCircle(const Vec2& p, const Vec2& dir,
                                   const Circle& c) {
    Vec2 q = p - c.m_center;
    float A = dir.LengthSquared();
    float B = 2.0 * q.Dot(dir);
    float C = q.LengthSquared() - c.m_radius * c.m_radius;
    float delta = B * B - 4 * A * C;

    if (delta <= std::numeric_limits<float>::epsilon()) {
        return std::nullopt;
    }

    float t1 = (-B + std::sqrt(delta)) / (2.0 * A);
    float t2 = (-B - std::sqrt(delta)) / (2.0 * A);

    if (t1 < 0) {
        if (t2 < 0) {
            return std::nullopt;
        }
        return t2;
    }

    if (t2 < 0) {
        return t1;
    }

    return std::min(t1, t2);
}

std::optional<HitResult> SweepRects(const Rect& r1, const Rect& r2,
                                    const Vec2& dir) {
    Rect r = r2;
    r.m_half_size += r1.m_half_size;

    return RaycastRect(r1.m_center, dir, r);
}

std::optional<float> SweepCircles(const Circle& c1, const Circle& c2,
                                  const Vec2& dir) {
    Circle c = c2;
    c.m_radius += c1.m_radius;
    return RaycastCircle(c1.m_center, dir, c);
}

std::optional<HitResult> SweepCircleRect(const Circle& c, const Rect& r,
                                         const Vec2& dir) {
    Vec2 top_left = r.m_center - r.m_half_size;
    Vec2 bottom_right = r.m_center + r.m_half_size;
    auto t1 =
        RayIntersect(c.m_center, dir, top_left - Vec2::Y_UNIT * c.m_radius,
                     Vec2::X_UNIT * r.m_half_size.w * 2);
    auto t2 =
        RayIntersect(c.m_center, dir, top_left - Vec2::X_UNIT * c.m_radius,
                     Vec2::Y_UNIT * r.m_half_size.h * 2);
    auto t3 =
        RayIntersect(c.m_center, dir, bottom_right + Vec2::Y_UNIT * c.m_radius,
                     -Vec2::X_UNIT * r.m_half_size.w * 2);
    auto t4 =
        RayIntersect(c.m_center, dir, bottom_right + Vec2::X_UNIT * c.m_radius,
                     -Vec2::Y_UNIT * r.m_half_size.h * 2);
    if (t1 && (t1->second < 0 || t1->second > 1)) {
        t1 = std::nullopt;
    }
    if (t2 && (t2->second < 0 || t2->second > 1)) {
        t2 = std::nullopt;
    }
    if (t3 && (t3->second < 0 || t3->second > 1)) {
        t3 = std::nullopt;
    }
    if (t4 && (t4->second < 0 || t4->second > 1)) {
        t4 = std::nullopt;
    }

    const std::array<Vec2, 4> rect_corners = {
        top_left, bottom_right, top_left + Vec2::X_UNIT * 2.0 * r.m_half_size.w,
        bottom_right - Vec2::X_UNIT * 2.0 * r.m_half_size.w};

    auto t5 = RaycastCircle(c.m_center, dir, {c.m_radius, rect_corners[0]});
    auto t6 = RaycastCircle(c.m_center, dir, {c.m_radius, rect_corners[1]});
    auto t7 = RaycastCircle(c.m_center, dir, {c.m_radius, rect_corners[2]});
    auto t8 = RaycastCircle(c.m_center, dir, {c.m_radius, rect_corners[3]});

    if (!t1 && !t2 && !t3 && !t4 && !t5 && !t6 && !t7 && !t8) {
        return std::nullopt;
    }

    constexpr std::array<HitType, 8> hit_types = {
        HitType::Top,
        HitType::Left,
        HitType::Bottom,
        HitType::Right,

        HitType::LeftTopCorner,
        HitType::RightBottomCorner,
        HitType::RightTopCorner,
        HitType::LeftBottomCorner,
    };

    static const std::array<Vec2, 4> hit_normals = {
        -Vec2::Y_UNIT,
        -Vec2::X_UNIT,
        Vec2::Y_UNIT,
        Vec2::X_UNIT,
    };

    std::array<float, 8> t_list = {
        t1 ? t1->first : std::numeric_limits<float>::infinity(),
        t2 ? t2->first : std::numeric_limits<float>::infinity(),
        t3 ? t3->first : std::numeric_limits<float>::infinity(),
        t4 ? t4->first : std::numeric_limits<float>::infinity(),
        t5 ? *t5 : std::numeric_limits<float>::infinity(),
        t6 ? *t6 : std::numeric_limits<float>::infinity(),
        t7 ? *t7 : std::numeric_limits<float>::infinity(),
        t8 ? *t8 : std::numeric_limits<float>::infinity(),
    };

    auto it = std::min_element(t_list.begin(), t_list.end());
    size_t idx = it - t_list.begin();
    if (idx < 4) {
        return HitResult{*it, hit_types[idx], hit_normals[idx]};
    }

    Vec2 final_center = c.m_center + dir * *it;
    Vec2 hit_normal = (final_center - rect_corners[idx - 4]).Normalize();

    return HitResult{*it, hit_types[idx], hit_normal};
}

PhysicsScene::PhysicsScene() {
    m_cached_hits.reserve(100);
}

void PhysicsScene::AddRect(const Rect& r) {
    m_rects.push_back(r);
}

void PhysicsScene::AddCircle(const Circle& c) {
    m_circle.push_back(c);
}

bool PhysicsScene::SweepByRect(const Rect& r1, const Vec2& dir, float dist,
                               HitResult* out_result, size_t out_size) {
    if (!out_result) {
        return false;
    }

    m_cached_hits.clear();

    for (auto& r2 : m_rects) {
        auto result = SweepRects(r1, r2, dir);
        if (!result || result->m_t > dist) {
            continue;
        }
        m_cached_hits.push_back(*result);
    }
    for (auto& c : m_circle) {
        auto result = SweepCircleRect(c, r1, -dir);
        if (!result || result->m_t > dist) {
            continue;
        }
        m_cached_hits.push_back(*result);
    }

    if (m_cached_hits.empty()) {
        return false;
    }

    std::sort(
        m_cached_hits.begin(), m_cached_hits.end(),
        [](const HitResult& a, const HitResult& b) { return a.m_t < b.m_t; });

    for (size_t i = 0; i < std::min(m_cached_hits.size(), out_size); ++i) {
        out_result[i] = m_cached_hits[i];
    }

    return true;
}

bool PhysicsScene::SweepByCircle(const Circle& c1, const Vec2& dir, float dist,
                                 HitResult* out_result, size_t out_size) {
    if (!out_result) {
        return false;
    }

    m_cached_hits.clear();

    for (auto& r : m_rects) {
        auto result = SweepCircleRect(c1, r, dir);
        if (!result || result->m_t > dist) {
            continue;
        }
        m_cached_hits.push_back(*result);
    }
    for (auto& c2 : m_circle) {
        auto result = SweepCircles(c1, c2, dir);
        if (!result || *result > dist) {
            continue;
        }

        Vec2 final_position = c1.m_center + dir * *result;
        m_cached_hits.push_back({*result, HitType::None,
                                 (final_position - c2.m_center).Normalize()});
    }

    if (m_cached_hits.empty()) {
        return false;
    }

    std::sort(
        m_cached_hits.begin(), m_cached_hits.end(),
        [](const HitResult& a, const HitResult& b) { return a.m_t < b.m_t; });

    for (size_t i = 0; i < std::min(m_cached_hits.size(), out_size); ++i) {
        out_result[i] = m_cached_hits[i];
    }

    return true;
}

void PhysicsScene::RenderDebug() const {
    if (!IsEnableDebugDraw()) {
        return;
    }

    auto& renderer = GAME_CONTEXT.m_renderer;
    for (auto& rect : m_rects) {
        renderer->DrawRect(rect, Color::Red);
    }
    for (auto& c : m_circle) {
        renderer->DrawCircle(c, Color::Red);
    }
}