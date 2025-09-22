#include "engine/physics.hpp"

#include "engine/context.hpp"
#include "engine/math.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

#include "engine/profile.hpp"

HitResult::HitResult(float t, Flags<HitType> f, Vec2 n, bool is_initial_overlap)
    : m_t(t),
      m_flags(f),
      m_normal(n),
      m_is_initial_overlap(is_initial_overlap) {}

Rect RectUnion(const Rect &r1, const Rect &r2) {
    Vec2 topleft1 = r1.m_center - r1.m_half_size;
    Vec2 bottomright1 = r1.m_center + r1.m_half_size;
    Vec2 topleft2 = r2.m_center - r2.m_half_size;
    Vec2 bottomright2 = r2.m_center + r2.m_half_size;

    float left = std::min(topleft1.x, topleft2.x);
    float right = std::min(bottomright1.x, bottomright1.x);
    float top = std::min(topleft1.y, topleft2.y);
    float bottom = std::min(bottomright2.y, bottomright2.y);

    Rect rect;
    rect.m_half_size.w = (right - left) * 0.5f;
    rect.m_half_size.h = (bottom - top) * 0.5f;
    rect.m_center.x = left + rect.m_half_size.x;
    rect.m_center.y = top + rect.m_half_size.y;
    return rect;
}

Vec2 NearestRectPoint(const Rect &r, const Vec2 &v) {
    Vec2 top_left = r.m_center - r.m_half_size;
    Vec2 bottom_right = r.m_center + r.m_half_size;

    return Vec2{Clamp(v.x, top_left.x, bottom_right.x),
                Clamp(v.y, top_left.y, bottom_right.y)};
}

Vec2 NearestCirclePoint(const Circle &c, const Vec2 &v) {
    return (v - c.m_center).Normalize() * c.m_radius + c.m_center;
}

bool IsPointInRect(const Vec2 &p, const Rect &r) {
    return p.x >= r.m_center.x - r.m_half_size.x &&
           p.x <= r.m_center.x + r.m_half_size.x &&
           p.y >= r.m_center.y - r.m_half_size.y &&
           p.y <= r.m_center.y + r.m_half_size.y;
}

bool IsRectsIntersect(const Rect &r1, const Rect &r2) {
    Rect r = r1;
    r.m_half_size += r2.m_half_size;

    return IsPointInRect(r2.m_center, r);
}

bool IsPointInCircle(const Vec2 &p, const Circle &c) {
    return (p - c.m_center).LengthSquared() <= c.m_radius * c.m_radius;
}

bool IsCirclesIntersect(const Circle &c1, const Circle &c2) {
    auto radius_sum = c1.m_radius + c2.m_radius;
    return (c1.m_center - c2.m_center).LengthSquared() <=
           radius_sum * radius_sum;
}

bool IsCircleRectIntersect(const Circle &c, const Rect &r) {
    Vec2 nearest_point = NearestRectPoint(r, c.m_center);
    return IsPointInCircle(nearest_point, c);
}

std::optional<std::pair<float, float> > RayIntersect(const Vec2 &p1,
                                                     const Vec2 &dir1,
                                                     const Vec2 &p2,
                                                     const Vec2 &dir2) {
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

std::optional<HitResult> RaycastRect(const Vec2 &p, const Vec2 &dir,
                                     const Rect &r) {
    if (IsPointInRect(p, r)) {
        HitResult result;
        result.m_is_initial_overlap = true;
        return result;
    }

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
    return HitResult{*it, hit_types[idx], hit_normals[idx], false};
}

std::optional<HitResult> RaycastCircle(const Vec2 &p, const Vec2 &dir,
                                       const Circle &c) {
    Vec2 q = p - c.m_center;
    double A = dir.LengthSquared();
    double B = 2.0 * q.Dot(dir);
    double C = q.LengthSquared() - c.m_radius * c.m_radius;
    double delta = B * B - 4 * A * C;

    if (delta <= std::numeric_limits<float>::epsilon()) {
        return std::nullopt;
    }

    double t1 = (-B + std::sqrt(delta)) / (2.0 * A);
    double t2 = (-B - std::sqrt(delta)) / (2.0 * A);

    if (t1 < 0 || t2 < 0) {
        if (t1 < 0 && t2 < 0) {
            return std::nullopt;
        }
        HitResult result;
        result.m_is_initial_overlap = true;
        return result;
    }

    auto t = std::min(t1, t2);
    HitResult result;
    result.m_t = t;
    result.m_normal = ((p + dir * t) - c.m_center).Normalize();
    return result;
}

std::optional<HitResult> SweepRects(const Rect &r1, const Rect &r2,
                                    const Vec2 &dir) {
    Rect r = r2;
    r.m_half_size += r1.m_half_size;

    return RaycastRect(r1.m_center, dir, r);
}

std::optional<HitResult> SweepCircles(const Circle &c1, const Circle &c2,
                                      const Vec2 &dir) {
    Circle c = c2;
    c.m_radius += c1.m_radius;
    return RaycastCircle(c1.m_center, dir, c);
}

std::optional<HitResult> SweepCircleRect(const Circle &c, const Rect &r,
                                         const Vec2 &dir) {
    Vec2 top_left = r.m_center - r.m_half_size;
    Vec2 bottom_right = r.m_center + r.m_half_size;

    Rect out_rect = r;
    out_rect.m_half_size += Vec2{c.m_radius, c.m_radius};

    // check whether raycast on rect
    if (auto t1 = RaycastRect(c.m_center, dir, out_rect)) {
        Vec2 final_position = c.m_center + dir * t1->m_t;
        if ((t1->m_flags & HitType::Left || t1->m_flags & HitType::Right) &&
            final_position.y >= top_left.y &&
            final_position.y <= bottom_right.y) {
            return t1;
        }
        if ((t1->m_flags & HitType::Top || t1->m_flags & HitType::Bottom) &&
            final_position.x >= top_left.x &&
            final_position.x <= bottom_right.x) {
            return t1;
        }
    }

    const std::array<Vec2, 4> rect_corners = {
        top_left, bottom_right, top_left + Vec2::X_UNIT * 2.0 * r.m_half_size.w,
        bottom_right - Vec2::X_UNIT * 2.0 * r.m_half_size.w};

    std::array<std::optional<HitResult>, 4> circle_hits;
    for (size_t i = 0; i < circle_hits.size(); ++i) {
        circle_hits[i] =
            RaycastCircle(c.m_center, dir, {rect_corners[i], c.m_radius});
    }

    bool not_hit_corner = std::all_of(circle_hits.begin(), circle_hits.end(),
                                      [](auto &hit) { return !hit; });
    if (not_hit_corner) {
        return std::nullopt;
    }

    // find initially overlapped circle
    if (auto it = std::find_if(
            circle_hits.begin(), circle_hits.end(),
            [](auto &hit) { return hit && hit->m_is_initial_overlap; });
        it != circle_hits.end()) {
        return *it;
    }

    constexpr std::array<HitType, 4> hit_types = {
        HitType::LeftTopCorner,
        HitType::RightBottomCorner,
        HitType::RightTopCorner,
        HitType::LeftBottomCorner,
    };

    auto it = std::min_element(circle_hits.begin(), circle_hits.end(),
                               [](auto &hit1, auto &hit2) {
                                   if (!hit1 && !hit2) {
                                       return false;
                                   }

                                   if (hit1 && !hit2) {
                                       return true;
                                   }

                                   if (!hit1 && hit2) {
                                       return false;
                                   }

                                   return hit1->m_t < hit2->m_t;
                               });

    size_t final_hit_index = it - circle_hits.begin();
    it->value().m_flags = hit_types[final_hit_index];
    return *it;
}

PhysicsShape::PhysicsShape(const Rect &r) : m_type(PhysicsShapeType::Rect) {
    m_rect = r;
}

PhysicsShape::PhysicsShape(const Circle &c) : m_type(PhysicsShapeType::Circle) {
    m_circle = c;
}

const Rect *PhysicsShape::AsRect() const {
    return m_type == PhysicsShapeType::Rect ? &m_rect : nullptr;
}

const Circle *PhysicsShape::AsCircle() const {
    return m_type == PhysicsShapeType::Circle ? &m_circle : nullptr;
}

PhysicsShapeType PhysicsShape::GetType() const {
    return m_type;
}

const Vec2 &PhysicsShape::GetPosition() const {
    return m_rect.m_center;
}

void PhysicsShape::MoveTo(const Vec2 &p) {
    m_rect.m_center = p;
}

void PhysicsShape::Move(const Vec2 &offset) {
    m_rect.m_center += offset;
}

PhysicsActor::PhysicsActor(Entity entity, const Rect &r, StorageType storage)
    : m_shape{r}, m_owner{entity}, m_storage_type{storage} {}

PhysicsActor::PhysicsActor(Entity entity, const Circle &c, StorageType storage)
    : m_shape{c}, m_owner{entity}, m_storage_type{storage} {}

PhysicsActor::PhysicsActor(Entity entity, const PhysicsShape &shape,
                           StorageType storage)
    : m_shape{shape}, m_owner{entity}, m_storage_type{storage} {}

const PhysicsShape &PhysicsActor::GetShape() const {
    return m_shape;
}

PhysicsActor::StorageType PhysicsActor::GetStorageType() const {
    return m_storage_type;
}

const Vec2 &PhysicsActor::GetPosition() const {
    return m_shape.GetPosition();
}

void PhysicsActor::SetCollisionLayer(CollisionGroup collision_group) {
    m_collision_layer = collision_group;
}

void PhysicsActor::SetCollisionMask(CollisionGroup collision_group) {
    m_collision_mask = collision_group;
}

void PhysicsActor::MoveTo(const Vec2 &position) {
    m_shape.MoveTo(position);
}

void PhysicsActor::Move(const Vec2 &offset) {
    m_shape.Move(offset);
}

void PhysicsScene::Chunks::getOverlapChunkRange(const Rect &bounding_box,
                                                Range2D<int> &out_chunk_range,
                                                Range2D<int> &out_tile_range) {
    const float ceil_constant = 0.5;

    auto top_left = bounding_box.m_center - bounding_box.m_half_size;
    auto bottom_right = bounding_box.m_center + bounding_box.m_half_size;
    int min_x = std::floor(top_left.x / (float)m_tile_size.w);
    int max_x =
        std::ceil(bottom_right.x / (float)m_tile_size.h + ceil_constant);
    int min_y = std::floor(top_left.y / (float)m_tile_size.w);
    int max_y =
        std::ceil(bottom_right.y / (float)m_tile_size.h + ceil_constant);

    out_chunk_range.m_x.m_begin = std::floor(min_x / (float)m_chunk_size.w);
    out_chunk_range.m_x.m_end =
        std::ceil(max_x / (float)m_chunk_size.w + ceil_constant);
    out_chunk_range.m_y.m_begin = std::floor(min_y / (float)m_chunk_size.h);
    out_chunk_range.m_y.m_end =
        std::ceil(max_y / (float)m_chunk_size.w + ceil_constant);

    out_tile_range.m_x.m_begin = std::floor(min_x % m_chunk_size.w);
    out_tile_range.m_y.m_begin = std::floor(min_y % m_chunk_size.h);
    out_tile_range.m_x.m_end = std::ceil(max_x % m_chunk_size.w);
    out_tile_range.m_y.m_end = std::ceil(max_y % m_chunk_size.h);
}

void PhysicsScene::Chunks::getTileRangeInCurrentChunk(
    const Range2D<int> &chunk_range, const Range2D<int> &tile_range, int x,
    int y, Range2D<int> &out_tile_range) {
    out_tile_range.m_x.m_begin =
        x == chunk_range.m_x.m_begin ? tile_range.m_x.m_begin : 0;
    out_tile_range.m_x.m_end =
        x == chunk_range.m_x.m_end - 1 ? tile_range.m_x.m_end : m_chunk_size.w;
    out_tile_range.m_y.m_begin =
        y == chunk_range.m_y.m_begin ? tile_range.m_y.m_begin : 0;
    out_tile_range.m_y.m_end =
        y == chunk_range.m_y.m_end - 1 ? tile_range.m_y.m_end : m_chunk_size.h;
}

PhysicsScene::PhysicsScene() {
    m_cached_sweep_results.reserve(100);
}

PhysicsActor *PhysicsScene::CreateActor(Entity entity,
                                        PhysicsActorInfoHandle info) {
    if (!info) {
        return nullptr;
    }

    PhysicsActor *actor{};
    if (info->m_is_rect) {
        actor = CreateActor(entity, info->m_rect);
    } else {
        actor = CreateActor(entity, info->m_circle);
    }

    if (actor) {
        actor->SetCollisionLayer(info->m_collision_layer);
        actor->SetCollisionMask(info->m_collision_mask);
        return actor;
    }

    return nullptr;
}

PhysicsActor *PhysicsScene::CreateActorInChunk(
    Entity entity, TilemapCollision *tilemap_collision, uint32_t layer,
    const PhysicsShape &shape) {
    if (!tilemap_collision) {
        return nullptr;
    }

    auto bounding = computeShapeBoundingBox(shape);

    auto &chunks = tilemap_collision->m_layers[layer];
    Range2D<int> chunk_range, tile_range;
    chunks.getOverlapChunkRange(bounding, chunk_range, tile_range);

    if (chunk_range.m_x.m_begin < 0 || chunk_range.m_y.m_begin < 0) {
        LOGE("chunk range begin is negative! ({}, {})", chunk_range.m_x.m_begin,
             chunk_range.m_y.m_begin);
        return nullptr;
    }

    if (!chunks.m_chunks.InRange(chunk_range.m_x.m_end - 1,
                                 chunk_range.m_y.m_end - 1)) {
        chunks.m_chunks.ExpandTo(chunk_range.m_x.m_end, chunk_range.m_y.m_end);
    }

    auto &actor =
        tilemap_collision->m_actors.emplace_back(std::make_unique<PhysicsActor>(
            entity, shape, PhysicsActor::StorageType::InChunk));

    for (int y = chunk_range.m_y.m_begin; y < chunk_range.m_y.m_end; y++) {
        for (int x = chunk_range.m_x.m_begin; x < chunk_range.m_x.m_end; x++) {
            auto &chunk = chunks.m_chunks.Get(x, y);
            if (chunk.GetSize() == 0) {
                chunk.ExpandTo(chunks.m_tile_size.w, chunks.m_tile_size.h);
            }
            Range2D<int> cur_tile_range;
            chunks.getTileRangeInCurrentChunk(chunk_range, tile_range, x, y,
                                              cur_tile_range);

            for (int sy = cur_tile_range.m_y.m_begin;
                 sy < cur_tile_range.m_y.m_end; sy++) {
                for (int sx = cur_tile_range.m_x.m_begin;
                     sx < cur_tile_range.m_x.m_end; sx++) {
                    auto &actors = chunk.Get(sx, sy);
                    actors.push_back(actor.get());
                }
            }
        }
    }

    return actor.get();
}

PhysicsScene::TilemapCollision *PhysicsScene::CreateTilemapCollision(
    const Vec2 &topleft) {
    return m_tilemap_collisions
        .emplace_back(std::make_unique<TilemapCollision>(topleft))
        .get();
}

PhysicsActor *PhysicsScene::CreateActor(Entity entity, const Circle &circle) {
    return m_actors
        .emplace_back(std::make_unique<PhysicsActor>(
            entity, circle, PhysicsActor::StorageType::Normal))
        .get();
}

PhysicsActor *PhysicsScene::CreateActor(Entity entity, const Rect &rect) {
    return m_actors
        .emplace_back(std::make_unique<PhysicsActor>(
            entity, rect, PhysicsActor::StorageType::Normal))
        .get();
}

void PhysicsScene::RemoveTilemapCollision(TilemapCollision *collision) {
    m_tilemap_collisions.erase(
        std::remove_if(m_tilemap_collisions.begin(), m_tilemap_collisions.end(),
                       [=](auto &value) { return value.get() == collision; }),
        m_tilemap_collisions.end());
}

void PhysicsScene::RemoveActor(PhysicsActor *actor) {
    if (!actor) return;

    if (actor->GetStorageType() == PhysicsActor::StorageType::Normal) {
        m_actors.erase(
            std::remove_if(m_actors.begin(), m_actors.end(),
                           [=](const std::unique_ptr<PhysicsActor> &o) {
                               return o.get() == actor;
                           }),
            m_actors.end());
    } else {
        for (auto &tilemap_collision : m_tilemap_collisions) {
            for (size_t i = 0; i < tilemap_collision->m_layers.size(); i++) {
                RemoveActorInChunk(tilemap_collision.get(), i, actor);
            }
        }
    }
}

void PhysicsScene::RemoveActorInChunk(TilemapCollision *tilemap_collision,
                                      uint32_t layer, PhysicsActor *actor) {
    if (!tilemap_collision || layer >= tilemap_collision->m_layers.size() ||
        !actor) {
        return;
    }

    auto &chunks = tilemap_collision->m_layers[layer];

    auto bounding = computeActorBoundingBox(*actor);
    Range2D<int> chunk_range, tile_range;
    chunks.getOverlapChunkRange(bounding, chunk_range, tile_range);
    for (int y = chunk_range.m_y.m_begin; y < chunk_range.m_y.m_end; y++) {
        for (int x = chunk_range.m_x.m_begin; x < chunk_range.m_x.m_end; x++) {
            if (!chunks.m_chunks.InRange(x, y)) {
                continue;
            }
            auto &chunk = chunks.m_chunks.Get(x, y);
            Range2D<int> cur_tile_range;
            chunks.getTileRangeInCurrentChunk(chunk_range, tile_range, x, y,
                                              cur_tile_range);

            for (int sy = cur_tile_range.m_y.m_begin;
                 sy < cur_tile_range.m_y.m_end; sy++) {
                for (int sx = cur_tile_range.m_x.m_begin;
                     sx < cur_tile_range.m_x.m_end; sx++) {
                    auto &actors = chunk.Get(sx, sy);
                    actors.erase(
                        std::remove(actors.begin(), actors.end(), actor),
                        actors.end());
                }
            }
        }
    }

    tilemap_collision->m_actors.erase(
        std::remove_if(tilemap_collision->m_actors.begin(),
                       tilemap_collision->m_actors.end(),
                       [=](auto &value) { return value.get() == actor; }),
        tilemap_collision->m_actors.end());
}

uint32_t PhysicsScene::Sweep(const PhysicsShape &shape, CollisionGroup mask,
                             const Vec2 &dir, float dist,
                             SweepResult *out_result, size_t out_size) {
    if (!out_result || out_size == 0) {
        return 0;
    }

    m_cached_sweep_results.clear();

    Rect sweep_rect = computeSweepBoundingBox(shape, dir, dist);

    // add a little skin for tolerance
    sweep_rect.m_half_size += {1, 1};

    // sweep normal actor
    for (auto &act : m_actors) {
        if (!checkNeedQuery(shape, mask, *act)) {
            continue;
        }
        Rect bounding_rect = computeActorBoundingBox(*act);
        if (IsRectsIntersect(bounding_rect, sweep_rect)) {
            auto result = sweepShape(shape, *act, dir);
            if (!result || result->m_t > dist) {
                continue;
            }
            SweepResult sweep_result;
            sweep_result.m_t = result->m_t;
            sweep_result.m_normal = result->m_normal;
            sweep_result.m_flags = result->m_flags;
            sweep_result.m_entity = act->GetEntity();
            sweep_result.m_is_initial_overlap = result->m_is_initial_overlap;
            sweep_result.m_actor = act.get();
            m_cached_sweep_results.push_back(sweep_result);
        }
    }

    // sweep chunk actor
    for (auto &tilemap_collision : m_tilemap_collisions) {
        Rect tilemap_rect;
        for (auto &layer : tilemap_collision->m_layers) {
            Vec2UI layer_size =
                layer.m_chunk_size * layer.m_tile_size *
                Vec2UI(layer.m_chunks.GetWidth(), layer.m_chunks.GetHeight());
            tilemap_rect.m_half_size.w =
                std::max<float>(layer_size.w, tilemap_rect.m_half_size.w);
            tilemap_rect.m_half_size.h =
                std::max<float>(layer_size.h, tilemap_rect.m_half_size.h);
        }

        tilemap_rect.m_half_size *= 0.5;
        tilemap_rect.m_center =
            tilemap_collision->m_topleft + tilemap_rect.m_half_size;

        if (!IsRectsIntersect(tilemap_rect, sweep_rect)) {
            continue;
        }

        for (auto &layer : tilemap_collision->m_layers) {
            Range2D<int> chunk_range, tile_range;
            layer.getOverlapChunkRange(sweep_rect, chunk_range, tile_range);

            for (int y = chunk_range.m_y.m_begin; y < chunk_range.m_y.m_end;
                 y++) {
                for (int x = chunk_range.m_x.m_begin; x < chunk_range.m_x.m_end;
                     x++) {
                    if (!layer.m_chunks.InRange(x, y)) {
                        continue;
                    }
                    auto &chunk = layer.m_chunks.Get(x, y);
                    Range2D<int> cur_tile_range;
                    layer.getTileRangeInCurrentChunk(chunk_range, tile_range, x,
                                                     y, cur_tile_range);

                    for (int sy = cur_tile_range.m_y.m_begin;
                         sy < cur_tile_range.m_y.m_end; sy++) {
                        for (int sx = cur_tile_range.m_x.m_begin;
                             sx < cur_tile_range.m_x.m_end; sx++) {
                            if (!chunk.InRange(sx, sy)) {
                                continue;
                            }

                            auto &actors = chunk.Get(sx, sy);
                            for (auto &act : actors) {
                                if (!checkNeedQuery(shape, mask, *act)) {
                                    continue;
                                }
                                std::optional<HitResult> result =
                                    sweepShape(shape, *act, dir);
                                if (!result || result->m_t > dist) {
                                    continue;
                                }
                                SweepResult sweep_result;
                                sweep_result.m_t = result->m_t;
                                sweep_result.m_normal = result->m_normal;
                                sweep_result.m_flags = result->m_flags;
                                sweep_result.m_entity = act->GetEntity();
                                sweep_result.m_is_initial_overlap =
                                    result->m_is_initial_overlap;
                                sweep_result.m_actor = act;
                                m_cached_sweep_results.push_back(sweep_result);
                            }
                        }
                    }
                }
            }
        }
    }

    if (m_cached_sweep_results.empty()) {
        return 0;
    }

    std::sort(
        m_cached_sweep_results.begin(), m_cached_sweep_results.end(),
        [](const HitResult &a, const HitResult &b) { return a.m_t < b.m_t; });

    size_t count = std::min(m_cached_sweep_results.size(), out_size);
    for (size_t i = 0; i < count; ++i) {
        out_result[i] = m_cached_sweep_results[i];
    }

    return count;
}

uint32_t PhysicsScene::Sweep(const PhysicsActor &actor, const Vec2 &dir,
                             float dist, SweepResult *out_result,
                             size_t out_size) {
    return Sweep(actor.GetShape(), actor.GetCollisionMask(), dir, dist,
                 out_result, out_size);
}

uint32_t PhysicsScene::Overlap(const PhysicsShape &shape, CollisionGroup mask,
                               OverlapResult *out_result, size_t out_size) {
    if (!out_result || out_size == 0) {
        return 0;
    }

    m_cached_overlaps_results.clear();

    auto bounding_box = computeShapeBoundingBox(shape);

    for (auto &act : m_actors) {
        if (!checkNeedQuery(shape, mask, *act)) {
            continue;
        }
        Rect bounding_rect = computeActorBoundingBox(*act);
        if (IsRectsIntersect(bounding_rect, bounding_box) &&
            Overlap(shape, act->GetShape())) {
            OverlapResult result;
            result.m_dst_entity = act->GetEntity();
            result.m_dst_actor = act.get();
            m_cached_overlaps_results.push_back(result);
        }
    }

    for (auto &tilemap_collision : m_tilemap_collisions) {
        Rect tilemap_rect;
        for (auto &layer : tilemap_collision->m_layers) {
            Vec2UI layer_size =
                layer.m_chunk_size * layer.m_tile_size *
                Vec2UI(layer.m_chunks.GetWidth(), layer.m_chunks.GetHeight());
            tilemap_rect.m_half_size.w =
                std::max<float>(layer_size.w, tilemap_rect.m_half_size.w);
            tilemap_rect.m_half_size.h =
                std::max<float>(layer_size.h, tilemap_rect.m_half_size.h);
        }

        tilemap_rect.m_half_size *= 0.5;
        tilemap_rect.m_center =
            tilemap_collision->m_topleft + tilemap_rect.m_half_size;

        if (!IsRectsIntersect(tilemap_rect, bounding_box)) {
            continue;
        }

        for (auto &layer : tilemap_collision->m_layers) {
            Range2D<int> chunk_range, tile_range;
            layer.getOverlapChunkRange(bounding_box, chunk_range, tile_range);

            for (int y = chunk_range.m_y.m_begin; y < chunk_range.m_y.m_end;
                 y++) {
                for (int x = chunk_range.m_x.m_begin; x < chunk_range.m_x.m_end;
                     x++) {
                    if (!layer.m_chunks.InRange(x, y)) {
                        continue;
                    }
                    auto &chunk = layer.m_chunks.Get(x, y);
                    Range2D<int> cur_tile_range;
                    layer.getTileRangeInCurrentChunk(chunk_range, tile_range, x,
                                                     y, cur_tile_range);

                    for (int sy = cur_tile_range.m_y.m_begin;
                         sy < cur_tile_range.m_y.m_end; sy++) {
                        for (int sx = cur_tile_range.m_x.m_begin;
                             sx < cur_tile_range.m_x.m_end; sx++) {
                            if (!chunk.InRange(sx, sy)) {
                                continue;
                            }

                            auto &actors = chunk.Get(sx, sy);
                            for (auto &act : actors) {
                                if (!(checkNeedQuery(shape, mask, *act) &&
                                      Overlap(shape, act->GetShape()))) {
                                    continue;
                                }
                                OverlapResult result;
                                result.m_dst_entity = act->GetEntity();
                                result.m_dst_actor = act;
                                m_cached_overlaps_results.push_back(result);
                            }
                        }
                    }
                }
            }
        }
    }

    if (m_cached_overlaps_results.empty()) {
        return 0;
    }

    size_t count = std::min(m_cached_overlaps_results.size(), out_size);
    for (size_t i = 0; i < count; ++i) {
        out_result[i] = m_cached_overlaps_results[i];
    }

    return count;
}

uint32_t PhysicsScene::Overlap(const PhysicsActor &actor,
                               OverlapResult *out_result, size_t out_size) {
    return Overlap(actor.GetShape(), actor.GetCollisionMask(), out_result,
                   out_size);
}

bool PhysicsScene::IsEnableDebugDraw() const {
    return m_should_debug_draw;
}

void PhysicsScene::RenderDebug() const {
    PROFILE_DEBUG_SECTION(__FUNCTION__);
    
    if (!IsEnableDebugDraw()) {
        return;
    }

    auto &renderer = CURRENT_CONTEXT.m_renderer;

    for (auto &actor : m_actors) {
        auto &shape = actor->GetShape();
        switch (shape.GetType()) {
            case PhysicsShapeType::Rect:
                renderer->DrawRect(*shape.AsRect(), Color::Red);
                break;
            case PhysicsShapeType::Circle:
                renderer->DrawCircle(*shape.AsCircle(), Color::Red);
                break;
            default:;
        }
    }

    // draw tiles
    for (auto &tilemap : m_tilemap_collisions) {
        for (auto &layer : tilemap->m_layers) {
            for (int x = 0; x < layer.m_chunks.GetWidth(); x++) {
                for (int y = 0; y < layer.m_chunks.GetHeight(); y++) {
                    auto &chunk = layer.m_chunks.Get(x, y);
                    for (int sx = 0; sx < chunk.GetWidth(); sx++) {
                        for (int sy = 0; sy < chunk.GetHeight(); sy++) {
                            for (auto &actor : chunk.Get(sx, sy)) {
                                if (!actor) {
                                    continue;
                                }

                                auto &shape = actor->GetShape();
                                switch (shape.GetType()) {
                                    case PhysicsShapeType::Rect:
                                        renderer->DrawRect(*shape.AsRect(),
                                                           Color::Red);
                                        break;
                                    case PhysicsShapeType::Circle:
                                        renderer->DrawCircle(*shape.AsCircle(),
                                                             Color::Red);
                                        break;
                                    default:;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // draw chunk area
    for (auto &tilemap_collision : m_tilemap_collisions) {
        for (auto &layer : tilemap_collision->m_layers) {
            for (size_t x = 0; x < layer.m_chunks.GetWidth(); x++) {
                for (size_t y = 0; y < layer.m_chunks.GetHeight(); y++) {
                    Rect rect;
                    rect.m_half_size.x =
                        layer.m_chunk_size.w * layer.m_tile_size.w * 0.5;
                    rect.m_half_size.y =
                        layer.m_chunk_size.h * layer.m_tile_size.h * 0.5;
                    rect.m_center = Vec2(x, y) * rect.m_half_size * 2.0 +
                                    rect.m_half_size +
                                    tilemap_collision->m_topleft;

                    renderer->DrawRect(rect, Color::Green);
                }
            }
        }
    }
}

Rect PhysicsScene::computeSweepBoundingBox(const Rect &r, const Vec2 &dir,
                                           float dist) const {
    Vec2 half_dir = dir * dist * 0.5f;
    Vec2 half_size = half_dir;
    half_size.x = std::abs(half_size.x);
    half_size.y = std::abs(half_size.y);
    return Rect{r.m_center + half_dir, r.m_half_size + half_size};
}

Rect PhysicsScene::computeSweepBoundingBox(const Circle &c, const Vec2 &dir,
                                           float dist) const {
    Rect rect = {
        c.m_center, {c.m_radius, c.m_radius}
    };
    return computeSweepBoundingBox(rect, dir, dist);
}

Rect PhysicsScene::computeSweepBoundingBox(const PhysicsShape &shape,
                                           const Vec2 &dir, float dist) const {
    if (shape.GetType() == PhysicsShapeType::Circle) {
        return computeSweepBoundingBox(*shape.AsCircle(), dir, dist);
    }
    if (shape.GetType() == PhysicsShapeType::Rect) {
        return computeSweepBoundingBox(*shape.AsRect(), dir, dist);
    }
    return {};
}

Rect PhysicsScene::computeSweepBoundingBox(const PhysicsActor &actor,
                                           const Vec2 &dir, float dist) const {
    return computeSweepBoundingBox(actor.GetShape(), dir, dist);
}

Rect PhysicsScene::computeActorBoundingBox(const PhysicsActor &actor) const {
    return computeShapeBoundingBox(actor.GetShape());
}

Rect PhysicsScene::computeShapeBoundingBox(const PhysicsShape &shape) const {
    switch (shape.GetType()) {
        case PhysicsShapeType::Unknown:
            return {};
        case PhysicsShapeType::Rect:
            return *shape.AsRect();
        case PhysicsShapeType::Circle: {
            auto &circle = *shape.AsCircle();
            return Rect{
                circle.m_center, {circle.m_radius, circle.m_radius}
            };
        }
    }

    return {};
}

std::optional<HitResult> PhysicsScene::sweepActor(const PhysicsActor &r1,
                                                  const PhysicsActor &r2,
                                                  const Vec2 &dir) const {
    auto &shape1 = r1.GetShape();
    if (shape1.GetType() == PhysicsShapeType::Rect) {
        return sweepGeometry(*shape1.AsRect(), r2, dir);
    }
    if (shape1.GetType() == PhysicsShapeType::Circle) {
        return sweepGeometry(*shape1.AsCircle(), r2, dir);
    }

    return std::nullopt;
}

std::optional<HitResult> PhysicsScene::sweepShape(const PhysicsShape &shape,
                                                  const PhysicsActor &actor,
                                                  const Vec2 &dir) const {
    if (shape.GetType() == PhysicsShapeType::Circle) {
        return sweepGeometry(*shape.AsCircle(), actor, dir);
    }
    if (shape.GetType() == PhysicsShapeType::Rect) {
        return sweepGeometry(*shape.AsRect(), actor, dir);
    }
    return std::nullopt;
}

std::optional<HitResult> PhysicsScene::sweepGeometry(const Circle &c,
                                                     const PhysicsActor &actor,
                                                     const Vec2 &dir) const {
    auto &shape = actor.GetShape();
    if (shape.GetType() == PhysicsShapeType::Rect) {
        return SweepCircleRect(c, *shape.AsRect(), dir);
    }
    if (shape.GetType() == PhysicsShapeType::Circle) {
        return SweepCircles(c, *shape.AsCircle(), dir);
    }
    return std::nullopt;
}

std::optional<HitResult> PhysicsScene::sweepGeometry(const Rect &r,
                                                     const PhysicsActor &actor,
                                                     const Vec2 &dir) const {
    auto &shape = actor.GetShape();
    if (shape.GetType() == PhysicsShapeType::Rect) {
        return SweepRects(r, *shape.AsRect(), dir);
    }
    if (shape.GetType() == PhysicsShapeType::Circle) {
        return SweepCircleRect(*shape.AsCircle(), r, -dir);
    }
    return std::nullopt;
}

bool PhysicsScene::Overlap(const PhysicsActor &r1,
                           const PhysicsActor &r2) const {
    auto &shape1 = r1.GetShape();
    auto &shape2 = r2.GetShape();
    return Overlap(shape1, shape2);
}

bool PhysicsScene::Overlap(const PhysicsShape &shape1,
                           const PhysicsShape &shape2) const {
    if (shape1.GetType() == PhysicsShapeType::Rect) {
        if (shape2.GetType() == PhysicsShapeType::Rect) {
            return IsRectsIntersect(*shape1.AsRect(), *shape2.AsRect());
        }
        if (shape2.GetType() == PhysicsShapeType::Circle) {
            return IsCircleRectIntersect(*shape2.AsCircle(), *shape1.AsRect());
        }
    } else if (shape1.GetType() == PhysicsShapeType::Circle) {
        if (shape2.GetType() == PhysicsShapeType::Rect) {
            return IsCircleRectIntersect(*shape1.AsCircle(), *shape2.AsRect());
        }
        if (shape2.GetType() == PhysicsShapeType::Circle) {
            return IsCirclesIntersect(*shape1.AsCircle(), *shape2.AsCircle());
        }
    }
    return false;
}

bool PhysicsScene::checkNeedQuery(const PhysicsShape &a, CollisionGroup mask,
                                  const PhysicsActor &b) const {
    if (&a == &b.GetShape()) {
        return false;
    }

    auto layer2 = b.GetCollisionLayer();

    return mask.CanCollision(layer2);
}
