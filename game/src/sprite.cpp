#include "sprite.hpp"

#include "context.hpp"
#include "sdl_call.hpp"
#include "transform.hpp"

#include <array>

void SpriteManager::Update() {
    auto& renderer = GAME_CONTEXT.m_renderer;
    auto& transform_manager = GAME_CONTEXT.m_transform_manager;
    for (auto& [entity, component ] : m_components) {
        if (!component.m_enable) {
            continue;
        }

        auto& sprite = component.m_component;
        
        const Transform* transform = transform_manager->Get(entity);
        if (!transform) {
            continue;
        }
        auto src_region = sprite->m_region;
        Vec2 half_size = sprite->m_size * 0.5f;

        std::array<Vec2, 3> pts;
        pts[0] = -half_size;  // top left
        pts[1] = {half_size.x, -half_size.y};  // top right
        pts[2] = {-half_size.x, half_size.y};  // bottom left

        if (sprite->m_flip & Flip::Horizontal &&
            sprite->m_flip & Flip::Vertical) {
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

        for (auto& pt : pts) {
            pt *= GAME_CONTEXT.m_camera.GetScale();
        }

        auto& m = transform->GetGlobalMat();
        for (auto& pt : pts) {
            Vec2 new_pt;
            new_pt.x = pt.x * m.Get(0, 0) + pt.y * m.Get(1, 0) +
                       m.Get(2, 0) * GAME_CONTEXT.m_camera.GetScale().x;
            new_pt.y = pt.x * m.Get(0, 1) + pt.y * m.Get(1, 1) +
                       m.Get(2, 1) * GAME_CONTEXT.m_camera.GetScale().y;
            pt = new_pt;

            pt += GAME_CONTEXT.m_camera.GetPosition();
        }

        renderer->DrawRectEx(*sprite->m_image, src_region, pts[0], pts[1], pts[2]);
    }
}