#include "physics.hpp"

#include "context.hpp"
#include "math.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

Vec2 NearestRectPoint(const Rect& r, const Vec2& v) {
    Vec2 top_left = r.m_center - r.m_half_size;
    Vec2 bottom_right = r.m_center + r.m_half_size;

    return Vec2{Clamp(v.x, top_left.x, bottom_right.x),
                Clamp(v.y, top_left.y, bottom_right.y)};
}

Vec2 NearestCirclePoint(const Circle& c, const Vec2& v) {
    return (v - c.m_center).Normalize() * c.m_radius + c.m_center;
}

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
    Vec2 nearest_point = NearestRectPoint(r, c.m_center);
    return IsPointInCircle(nearest_point, c);
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

    auto t5 = RaycastCircle(c.m_center, dir, {rect_corners[0], c.m_radius});
    auto t6 = RaycastCircle(c.m_center, dir, {rect_corners[1], c.m_radius});
    auto t7 = RaycastCircle(c.m_center, dir, {rect_corners[2], c.m_radius});
    auto t8 = RaycastCircle(c.m_center, dir, {rect_corners[3], c.m_radius});

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

PhysicsActor::PhysicsActor(const Rect& r, StorageType storage)
    : m_type{ShapeType::Rect}, m_rect{r}, m_storage_type{storage} {}

PhysicsActor::PhysicsActor(const Circle& c, StorageType storage)
    : m_type{ShapeType::Circle}, m_circle{c}, m_storage_type{storage} {}

const Rect* PhysicsActor::AsRect() const {
    return m_type == ShapeType::Rect ? &m_rect : nullptr;
}

const Circle* PhysicsActor::AsCircle() const {
    return m_type == ShapeType::Circle ? &m_circle : nullptr;
}

PhysicsActor::ShapeType PhysicsActor::GetShapeType() const {
    return m_type;
}

PhysicsActor::StorageType PhysicsActor::GetStorageType() const {
    return m_storage_type;
}

const Vec2& PhysicsActor::GetPosition() const {
    return m_rect.m_center;
}

PhysicsScene::PhysicsScene() {
    m_cached_hits.reserve(100);
}

PhysicsActor* PhysicsScene::CreateActorInChunk(size_t chunk_x, size_t chunk_y,
                                               size_t x, size_t y,
                                               const Rect& rect) {
    const auto& game_config = GAME_CONTEXT.GetGameConfig();
    if (x >= game_config.m_tile_in_chunk_size_w ||
        y >= game_config.m_tile_in_chunk_size_h) {
        LOGE("does tile position ({}, {}) out of range ({}, {}) ?", x, y,
             game_config.m_tile_in_chunk_size_w,
             game_config.m_tile_in_chunk_size_h);
        return nullptr;
    }

    return setActor2Chunk(chunk_x, chunk_y, x, y,
                          std::make_unique<PhysicsActor>(
                              rect, PhysicsActor::StorageType::InChunk));
}

PhysicsActor* PhysicsScene::CreateActorInChunk(size_t chunk_x, size_t chunk_y,
                                               size_t x, size_t y,
                                               const Circle& circle) {
    const auto& game_config = GAME_CONTEXT.GetGameConfig();
    if (x >= game_config.m_tile_in_chunk_size_w ||
        y >= game_config.m_tile_in_chunk_size_h) {
        LOGE("does tile position ({}, {}) out of range ({}, {}) ?", x, y,
             game_config.m_tile_in_chunk_size_w,
             game_config.m_tile_in_chunk_size_h);
        return nullptr;
    }

    return setActor2Chunk(chunk_x, chunk_y, x, y,
                          std::make_unique<PhysicsActor>(
                              circle, PhysicsActor::StorageType::InChunk));
}

PhysicsActor* PhysicsScene::CreateActorInChunk(const Vec2& center,
                                               const Rect& rect) {
    auto actors = getActorStoreInChunk(center, true);
    if (!actors) {
        return nullptr;
    }

    return actors
        ->emplace_back(std::make_unique<PhysicsActor>(
            rect, PhysicsActor::StorageType::InChunk))
        .get();
}

PhysicsActor* PhysicsScene::CreateActorInChunk(const Vec2& center,
                                               const Circle& c) {
    auto actors = getActorStoreInChunk(center, true);
    if (!actors) {
        return nullptr;
    }

    return actors
        ->emplace_back(std::make_unique<PhysicsActor>(
            c, PhysicsActor::StorageType::InChunk))
        .get();
}

PhysicsActor* PhysicsScene::CreateActor(const Circle& circle) {
    return m_actors
        .emplace_back(std::make_unique<PhysicsActor>(
            circle, PhysicsActor::StorageType::Normal))
        .get();
}

PhysicsActor* PhysicsScene::CreateActor(const Rect& rect) {
    return m_actors
        .emplace_back(std::make_unique<PhysicsActor>(
            rect, PhysicsActor::StorageType::Normal))
        .get();
}

void PhysicsScene::RemoveActor(PhysicsActor* actor) {
    if (!actor) return;

    if (actor->GetStorageType() == PhysicsActor::StorageType::Normal) {
        m_actors.erase(
            std::remove_if(m_actors.begin(), m_actors.end(),
                           [=](const std::unique_ptr<PhysicsActor>& o) {
                               return o.get() == actor;
                           }),
            m_actors.end());
    } else {
        auto actors = getActorStoreInChunk(actor->GetPosition(), false);
        if (!actors) {
            return;
        }

        actors->erase(
            std::remove_if(actors->begin(), actors->end(),
                           [=](const std::unique_ptr<PhysicsActor>& o) {
                               return o.get() == actor;
                           }),
            actors->end());
    }
}

bool PhysicsScene::Sweep(const PhysicsActor& actor, const Vec2& dir, float dist,
                         HitResult* out_result, size_t out_size) {
    if (!out_result) {
        return false;
    }

    m_cached_hits.clear();

    auto sweep_rect = computeSweepBoundingBox(actor, dir, dist);

    // add a little skin for tolerance
    sweep_rect.m_half_size += {1, 1};

    for (auto& act : m_actors) {
        Rect bounding_rect = computeActorBoundingBox(*act);
        if (IsRectsIntersect(bounding_rect, sweep_rect)) {
            auto result = sweepActor(actor, *act, dir);
            if (!result || result->m_t > dist) {
                continue;
            }
            m_cached_hits.push_back(*result);
        }
    }

    const auto& game_config = GAME_CONTEXT.GetGameConfig();

    auto top_left = sweep_rect.m_center - sweep_rect.m_half_size;
    auto bottom_right = sweep_rect.m_center + sweep_rect.m_half_size;
    int min_x = std::floor(top_left.x / (float)game_config.m_tile_size_w);
    int max_x = std::round(bottom_right.x / (float)game_config.m_tile_size_h + 0.5);
    int min_y = std::floor(top_left.y / (float)game_config.m_tile_size_w);
    int max_y = std::round(bottom_right.y / (float)game_config.m_tile_size_h + 0.5);

    int min_chunk_x =
        std::floor(min_x / (float)game_config.m_tile_in_chunk_size_w);
    int max_chunk_x =
        std::round(max_x / (float)game_config.m_tile_in_chunk_size_w + 0.5);
    int min_chunk_y =
        std::floor(min_y / (float)game_config.m_tile_in_chunk_size_h);
    int max_chunk_y =
        std::round(max_y / (float)game_config.m_tile_in_chunk_size_h + 0.5);

    int min_tile_x = std::floor(min_x % game_config.m_tile_in_chunk_size_w);
    int min_tile_y = std::floor(min_y % game_config.m_tile_in_chunk_size_h);
    int max_tile_x = std::ceil(max_x % game_config.m_tile_in_chunk_size_w);
    int max_tile_y = std::ceil(max_y % game_config.m_tile_in_chunk_size_h);

    for (int y = min_chunk_y; y < max_chunk_y; y++) {
        for (int x = min_chunk_x; x < max_chunk_x; x++) {
            if (!m_chunks.InRange(x, y)) {
                continue;
            }
            auto& chunk = m_chunks.Get(x, y);
            if (!chunk) {
                continue;
            }

            int x1 = x == min_chunk_x ? min_tile_x : 0;
            int x2 = x == max_chunk_x - 1 ? max_tile_x
                                          : game_config.m_tile_in_chunk_size_w;
            int y1 = y == min_chunk_y ? min_tile_y : 0;
            int y2 = y == max_chunk_y - 1 ? max_tile_y
                                          : game_config.m_tile_in_chunk_size_h;

            for (int sy = y1; sy < y2; sy++) {
                for (int sx = x1; sx < x2; sx++) {
                    if (!chunk->InRange(sx, sy)) {
                        continue;
                    }

                    auto& actors = chunk->Get(sx, sy);
                    for (auto& act : actors) {
                        std::optional<HitResult> result =
                            sweepActor(actor, *act, dir);
                        if (!result || result->m_t > dist) {
                            continue;
                        }
                        m_cached_hits.push_back(*result);
                    }
                }
            }
        }
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

    for (auto& actor : m_actors) {
        switch (actor->GetShapeType()) {
            case PhysicsActor::ShapeType::Rect:
                renderer->DrawRect(*actor->AsRect(), Color::Red);
                break;
            case PhysicsActor::ShapeType::Circle:
                renderer->DrawCircle(*actor->AsCircle(), Color::Red);
                break;
            default:;
        }
    }

    for (int x = 0; x < m_chunks.GetWidth(); x++) {
        for (int y = 0; y < m_chunks.GetHeight(); y++) {
            auto& chunk = m_chunks.Get(x, y);
            if (!chunk) {
                continue;
            }

            for (int sx = 0; sx < chunk->GetWidth(); sx++) {
                for (int sy = 0; sy < chunk->GetHeight(); sy++) {
                    for (auto& actor : chunk->Get(sx, sy)) {
                        if (!actor) {
                            continue;
                        }
                        switch (actor->GetShapeType()) {
                            case PhysicsActor::ShapeType::Rect:
                                renderer->DrawRect(*actor->AsRect(),
                                                   Color::Red);
                                break;
                            case PhysicsActor::ShapeType::Circle:
                                renderer->DrawCircle(*actor->AsCircle(),
                                                     Color::Red);
                                break;
                            default:;
                        }
                    }
                }
            }
        }
    }

    auto& game_config = GAME_CONTEXT.GetGameConfig();
    for (size_t x = 0; x < m_chunks.GetWidth(); x++) {
        for (size_t y = 0; y < m_chunks.GetHeight(); y++) {
            Rect rect;
            rect.m_half_size.x = game_config.m_tile_in_chunk_size_w *
                                 game_config.m_tile_size_w * 0.5;
            rect.m_half_size.y = game_config.m_tile_in_chunk_size_h *
                                 game_config.m_tile_size_h * 0.5;
            rect.m_center = Vec2(x, y) * rect.m_half_size * 2.0 + rect.m_half_size;

            renderer->DrawRect(rect, Color::Green);
        }
    }
}

PhysicsScene::Chunk& PhysicsScene::ensureChunk(size_t x, size_t y) {
    if (!m_chunks.InRange(x, y)) {
        m_chunks.Resize(x + 1, y + 1);
    }

    auto& chunk = m_chunks.Get(x, y);
    if (!chunk) {
        chunk = std::make_unique<Chunk>();
    }

    return *chunk;
}

PhysicsActor* PhysicsScene::setActor2Chunk(
    size_t chunk_x, size_t chunk_y, size_t x, size_t y,
    std::unique_ptr<PhysicsActor>&& actor) {
    const auto& game_config = GAME_CONTEXT.GetGameConfig();
    if (x >= game_config.m_tile_in_chunk_size_w ||
        y >= game_config.m_tile_in_chunk_size_h) {
        LOGE("does tile position ({}, {}) out of range {} ?", x, y,
             game_config.m_tile_in_chunk_size_w,
             game_config.m_tile_in_chunk_size_h);
        return nullptr;
    }

    auto& chunk = ensureChunk(chunk_x, chunk_y);

    auto& actors = chunk.Get(x, y);
    actors.push_back(std::move(actor));
    return chunk.Get(x, y).back().get();
}

Rect PhysicsScene::computeSweepBoundingBox(const Rect& r, const Vec2& dir,
                                           float dist) const {
    Vec2 half_dir = dir * dist * 0.5f;
    Vec2 half_size = half_dir;
    half_size.x = std::abs(half_size.x);
    half_size.y = std::abs(half_size.y);
    return Rect{r.m_center + half_dir, r.m_half_size + half_size};
}

Rect PhysicsScene::computeSweepBoundingBox(const Circle& c, const Vec2& dir,
                                           float dist) const {
    Rect rect = {
        c.m_center, {c.m_radius, c.m_radius}
    };
    return computeSweepBoundingBox(rect, dir, dist);
}

Rect PhysicsScene::computeSweepBoundingBox(const PhysicsActor& actor,
                                           const Vec2& dir, float dist) const {
    if (actor.GetShapeType() == PhysicsActor::ShapeType::Circle) {
        return computeSweepBoundingBox(*actor.AsCircle(), dir, dist);
    }
    if (actor.GetShapeType() == PhysicsActor::ShapeType::Rect) {
        return computeSweepBoundingBox(*actor.AsRect(), dir, dist);
    }
    return {};
}

std::vector<std::unique_ptr<PhysicsActor>>* PhysicsScene::getActorStoreInChunk(
    const Vec2& actor_center, bool ensure) {
    const auto& game_config = GAME_CONTEXT.GetGameConfig();
    int x = std::floor(actor_center.x / game_config.m_tile_size_w);
    int y = std::floor(actor_center.y / game_config.m_tile_size_h);
    int chunk_x = x / game_config.m_tile_in_chunk_size_w;
    int chunk_y = y / game_config.m_tile_in_chunk_size_h;
    int tile_x = x % game_config.m_tile_in_chunk_size_w;
    int tile_y = y % game_config.m_tile_in_chunk_size_h;

    if (!m_chunks.InRange(chunk_x, chunk_y)) {
        if (!ensure) {
            return nullptr;
        }
        m_chunks.ExpandTo(chunk_x + 1, chunk_y + 1);
    }

    auto& chunk = m_chunks.Get(chunk_x, chunk_y);
    if (!chunk) {
        if (!ensure) {
            return nullptr;
        }
        chunk = std::make_unique<Chunk>();
        chunk->ExpandTo(game_config.m_tile_in_chunk_size_w,
                        game_config.m_tile_in_chunk_size_h);
    }
    if (!chunk->InRange(tile_x, tile_y)) {
        return nullptr;
    }

    return &chunk->Get(tile_x, tile_y);
}

Rect PhysicsScene::computeActorBoundingBox(const PhysicsActor& actor) const {
    switch (actor.GetShapeType()) {
        case PhysicsActor::ShapeType::Unknown:
            return {};
        case PhysicsActor::ShapeType::Rect:
            return *actor.AsRect();
        case PhysicsActor::ShapeType::Circle: {
            auto& circle = *actor.AsCircle();
            return Rect{
                circle.m_center, {circle.m_radius, circle.m_radius}
            };
        }
    }

    return {};
}

std::optional<HitResult> PhysicsScene::sweepActor(const PhysicsActor& r1,
                                                  const PhysicsActor& r2,
                                                  const Vec2& dir) const {
    if (r1.GetShapeType() == PhysicsActor::ShapeType::Rect) {
        if (r2.GetShapeType() == PhysicsActor::ShapeType::Rect) {
            return SweepRects(*r1.AsRect(), *r2.AsRect(), dir);
        }
        if (r2.GetShapeType() == PhysicsActor::ShapeType::Circle) {
            return SweepCircleRect(*r2.AsCircle(), *r1.AsRect(), -dir);
        }
    } else if (r1.GetShapeType() == PhysicsActor::ShapeType::Circle) {
        if (r2.GetShapeType() == PhysicsActor::ShapeType::Rect) {
            return SweepCircleRect(*r1.AsCircle(), *r2.AsRect(), dir);
        }
        if (r2.GetShapeType() == PhysicsActor::ShapeType::Circle) {
            auto result = SweepCircles(*r1.AsCircle(), *r2.AsCircle(), dir);
            if (result) {
                Vec2 final_position =
                    r1.AsCircle()->m_center + dir * result.value();
                return HitResult{
                    result.value(), HitType::None,
                    (final_position - r2.AsCircle()->m_center).Normalize()};
            }
        }
    }

    return std::nullopt;
}