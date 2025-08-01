#include "sprite.hpp"

#include "context.hpp"
#include "sdl_call.hpp"
#include "transform.hpp"

#include <array>

void SpriteManager::Update() {
    auto& renderer = GAME_CONTEXT.m_renderer;
    auto& transform_manager = GAME_CONTEXT.m_transform_manager;
    for (auto& [entity, sprite] : m_components) {
        const Transform* transform = transform_manager->Get(entity);
        if (!transform) {
            continue;
        }
        auto src_region = sprite->m_region;
        Vec2 half_size = sprite->m_size * 0.5f;

        std::array<Vec2, 3> pts;
        pts[0] = -half_size;                   // top left
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

        auto& m = transform->GetGlobalMat();
        for (auto& pt : pts) {
            Vec2 new_pt;
            new_pt.x = pt.x * m.Get(0, 0) + pt.y * m.Get(1, 0) + m.Get(2, 0);
            new_pt.y = pt.x * m.Get(0, 1) + pt.y * m.Get(1, 1) + m.Get(2, 1);
            pt = new_pt;
        }

        SDL_FPoint topleft, topright, bottomleft;
        topleft.x = pts[0].x;
        topleft.y = pts[0].y;
        topright.x = pts[1].x;
        topright.y = pts[1].y;
        bottomleft.x = pts[2].x;
        bottomleft.y = pts[2].y;

        SDL_FRect src_rect;
        src_rect.x = src_region.m_topleft.x;
        src_rect.y = src_region.m_topleft.y;
        src_rect.w = src_region.m_size.w;
        src_rect.h = src_region.m_size.h;

        SDL_CALL(SDL_RenderTextureAffine(
            renderer->GetRenderer(), sprite->m_image->GetTexture(), &src_rect,
            &topleft, &topright, &bottomleft));
    }
}