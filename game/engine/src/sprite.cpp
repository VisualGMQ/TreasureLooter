#include "engine/sprite.hpp"

#include "engine/context.hpp"
#include "engine/sdl_call.hpp"
#include "engine/transform.hpp"

#include <array>

#include "engine/profile.hpp"

void SpriteManager::SubmitDrawCommand(Entity entity) {
    PROFILE_SECTION();

    auto sprite = Get(entity);
    TL_RETURN_IF_FALSE(sprite && sprite->m_image && IsEnable(entity));

    auto& renderer = CURRENT_CONTEXT.m_renderer;
    auto& transform_manager = CURRENT_CONTEXT.m_transform_manager;
    const Transform* transform = transform_manager->Get(entity);
    TL_RETURN_IF_FALSE(transform);

    auto src_region = sprite->m_region;
    if (src_region.m_size.w == 0 || src_region.m_size.h == 0) {
        src_region.m_size = sprite->m_image->GetSize();
    }

    Vec2 half_size = sprite->m_region.m_size * 0.5f;

    if (half_size.x == 0 || half_size.y == 0) {
        half_size = sprite->m_image->GetSize() * 0.5;
    }

    std::array<Vec2, 3> pts;
    pts[0] = -half_size;                   // top left
    pts[1] = {half_size.x, -half_size.y};  // top right
    pts[2] = {-half_size.x, half_size.y};  // bottom left

    if (sprite->m_flip & Flip::Horizontal && sprite->m_flip & Flip::Vertical) {
        pts[0] = half_size;
        pts[1] = {-half_size.x, half_size.y};
        pts[2] = {half_size.x, -half_size.y};
    } else if (sprite->m_flip & Flip::Horizontal) {
        pts[0] = {half_size.x, -half_size.y};
        pts[1] = {-half_size.x, -half_size.y};
        pts[2] = {half_size.x, half_size.y};
    } else if (sprite->m_flip & Flip::Vertical) {
        pts[0] = {-half_size.x, half_size.y};
        pts[1] = {half_size.x, half_size.y};
        pts[2] = -half_size;
    }

    auto& m = transform->GetGlobalMat();
    for (auto& pt : pts) {
        Vec2 pt_with_anchor = pt;
        pt_with_anchor -= sprite->m_anchor;
        Vec2 new_pt;
        new_pt.x = pt_with_anchor.x * m.Get(0, 0) +
                   pt_with_anchor.y * m.Get(1, 0) + m.Get(2, 0);
        new_pt.y = pt_with_anchor.x * m.Get(0, 1) +
                   pt_with_anchor.y * m.Get(1, 1) + m.Get(2, 1);
        pt = new_pt;
    }

    auto draw_order = CURRENT_CONTEXT.m_draw_order_manager->Get(entity);
    float z_order = draw_order ? draw_order->GetGlobalOrder() : 0;

    renderer->DrawImageEx(*sprite->m_image, src_region, pts[0], pts[1], pts[2],
                          sprite->m_color, z_order, true, transform->m_position.y);
}
