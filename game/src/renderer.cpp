#include "renderer.hpp"

#include "camera.hpp"
#include "context.hpp"
#include "image.hpp"
#include "log.hpp"
#include "sdl_call.hpp"

Renderer::Renderer(Window &window) {
    m_renderer = SDL_CreateRenderer(window.GetWindow(), nullptr);
    if (!m_renderer) {
        LOGE("create SDL renderer failed: {}", SDL_GetError());
    }
    SDL_CALL(SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND));
}

Renderer::~Renderer() {
    SDL_DestroyRenderer(m_renderer);
}

void Renderer::SetClearColor(const Color &c) {
    m_clear_color.r = c.r * 255;
    m_clear_color.g = c.g * 255;
    m_clear_color.b = c.b * 255;
    m_clear_color.a = c.a * 255;
}

void Renderer::DrawLine(const Vec2 &p1, const Vec2 &p2, const Color &color,
                        bool use_camera) {
    setRenderColor(color);

    Vec2 dp1 = p1;
    Vec2 dp2 = p2;
    if (use_camera) {
        transformByCamera(GAME_CONTEXT.m_camera, &dp1, nullptr);
        transformByCamera(GAME_CONTEXT.m_camera, &dp2, nullptr);
    }
    SDL_CALL(SDL_RenderLine(m_renderer, p1.x, p1.y, p2.x, p2.y));
}

void Renderer::DrawRect(const Rect &r, const Color &c, bool use_camera) {
    setRenderColor(c);

    Rect dst = r;
    if (use_camera) {
        transformByCamera(GAME_CONTEXT.m_camera, &dst.m_center,
                          &dst.m_half_size);
    }
    Vec2 tl = dst.m_center - dst.m_half_size;
    SDL_FRect rect{
        tl.x, tl.y, dst.m_half_size.w * 2.0f,
        dst.m_half_size.h * 2.0f
    };
    SDL_CALL(SDL_RenderRect(m_renderer, &rect));
}

void Renderer::DrawCircle(const Circle &c, const Color &color,
                          uint32_t fragment, bool use_camera) {
    float angle_step = 2 * PI / fragment;
    Vec2 p = c.m_center + Vec2::X_UNIT * c.m_radius;
    setRenderColor(color);
    auto &camera = GAME_CONTEXT.m_camera;
    if (use_camera) {
        transformByCamera(camera, &p, nullptr);
    }
    for (int i = 1; i <= fragment; i++) {
        Vec2 new_p = c.m_center;
        float angle = angle_step * i;
        new_p.x += c.m_radius * std::cos(angle);
        new_p.y += c.m_radius * std::sin(angle);
        if (use_camera) {
            transformByCamera(camera, &new_p, nullptr);
        }
        SDL_RenderLine(m_renderer, p.x, p.y, new_p.x, new_p.y);
        p = new_p;
    }
}

void Renderer::FillRect(const Rect &r, const Color &c, bool use_camera) {
    setRenderColor(c);
    Rect dst = r;
    if (use_camera) {
        transformByCamera(GAME_CONTEXT.m_camera, &dst.m_center,
                          &dst.m_half_size);
    }
    Vec2 tl = dst.m_center - dst.m_half_size;
    SDL_FRect rect{
        tl.x, tl.y, dst.m_half_size.w * 2.0f,
        dst.m_half_size.h * 2.0f
    };
    SDL_CALL(SDL_RenderFillRect(m_renderer, &rect));
}

void Renderer::DrawImage(const Image &image, const Region &src,
                         const Region &dst, Degrees rotation,
                         const Vec2 &center, Flags<Flip> flip,
                         bool use_camera) {
    Rect dst_region;
    dst_region.m_half_size = dst.m_size * 0.5;
    dst_region.m_center = dst.m_topleft + dst_region.m_half_size;

    if (use_camera) {
        transformByCamera(GAME_CONTEXT.m_camera, &dst_region.m_center,
                          &dst_region.m_half_size);
    }

    SDL_FRect src_rect, dst_rect;
    src_rect.x = src.m_topleft.x;
    src_rect.y = src.m_topleft.y;
    src_rect.w = src.m_size.w;
    src_rect.h = src.m_size.h;

    auto top_left = dst_region.m_center - dst_region.m_half_size;
    dst_rect.x = top_left.x;
    dst_rect.y = top_left.y;
    dst_rect.w = dst_region.m_half_size.w * 2.0f;
    dst_rect.h = dst_region.m_half_size.h * 2.0f;

    SDL_FPoint sdl_center;
    Vec2 rot_center;
    if (use_camera) {
        transformByCamera(GAME_CONTEXT.m_camera, &rot_center, nullptr);
    }
    sdl_center.x = rot_center.x;
    sdl_center.y = rot_center.y;
    SDL_CALL(SDL_RenderTextureRotated(m_renderer, image.GetTexture(), &src_rect,
        &dst_rect, rotation.Value(), &sdl_center,
        static_cast<SDL_FlipMode>(flip.Value())));
}

void Renderer::DrawRectEx(const Image &image, const Region &src,
                          const Vec2 &topleft, const Vec2 &topright,
                          const Vec2 &bottomleft, bool use_camera) {
    SDL_FRect rect = {
        src.m_topleft.x, src.m_topleft.y, src.m_size.w,
        src.m_size.h
    };
    Vec2 tl{topleft.x, topleft.y}, tr{topright.x, topright.y},
            bl{bottomleft.x, bottomleft.y};
    if (use_camera) {
        transformByCamera(GAME_CONTEXT.m_camera, &tl, nullptr);
        transformByCamera(GAME_CONTEXT.m_camera, &tr, nullptr);
        transformByCamera(GAME_CONTEXT.m_camera, &bl, nullptr);
    }

    SDL_FPoint sdl_tl{tl.x, tl.y}, sdl_tr{tr.x, tr.y}, sdl_bl{bl.x, bl.y};
    SDL_CALL(SDL_RenderTextureAffine(m_renderer, image.GetTexture(), &rect, &sdl_tl,
        &sdl_tr, &sdl_bl));
}

void Renderer::Clear() {
    SDL_CALL(SDL_SetRenderDrawColor(m_renderer, m_clear_color.r,
        m_clear_color.g, m_clear_color.b,
        m_clear_color.a));
    SDL_CALL(SDL_RenderClear(m_renderer));
}

void Renderer::Present() {
    SDL_CALL(SDL_RenderPresent(m_renderer));
}

SDL_Renderer *Renderer::GetRenderer() const {
    return m_renderer;
}

void Renderer::setRenderColor(const Color &c) {
    SDL_CALL(SDL_SetRenderDrawColor(m_renderer, c.r * 255, c.g * 255, c.b * 255,
        c.a * 255));
}

void Renderer::transformByCamera(const Camera &camera, Vec2 *center,
                                 Vec2 *size) const {
    if (center) {
        auto window_size = GAME_CONTEXT.m_window->GetWindowSize();
        *center = (*center - camera.GetPosition()) * camera.GetScale() + window_size * 0.5;
    }
    if (size) {
        *size *= camera.GetScale();
    }
}
