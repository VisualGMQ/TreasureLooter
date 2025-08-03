#include "renderer.hpp"

#include "image.hpp"
#include "log.hpp"
#include "sdl_call.hpp"

Renderer::Renderer(Window& window) {
    m_renderer = SDL_CreateRenderer(window.GetWindow(), nullptr);
    if (!m_renderer) {
        LOGE("create SDL renderer failed: {}", SDL_GetError());
    }
    SDL_CALL(SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND));
}

Renderer::~Renderer() {
    SDL_DestroyRenderer(m_renderer);
}

void Renderer::SetClearColor(const Color& c) {
    m_clear_color.r = c.r * 255;
    m_clear_color.g = c.g * 255;
    m_clear_color.b = c.b * 255;
    m_clear_color.a = c.a * 255;
}

void Renderer::DrawLine(const Vec2& p1, const Vec2& p2, const Color& color) {
    setRenderColor(color);
    SDL_CALL(SDL_RenderLine(m_renderer, p1.x, p1.y, p2.x, p2.y));
}

void Renderer::DrawRect(const Rect& r, const Color& c) {
    setRenderColor(c);
    Vec2 tl = r.m_center - r.m_half_size;
    SDL_FRect rect{tl.x, tl.y, r.m_half_size.w * 2.0f, r.m_half_size.h * 2.0f};
    SDL_CALL(SDL_RenderRect(m_renderer, &rect));
}

void Renderer::DrawCircle(const Circle& c, const Color& color, uint32_t fragment) {
    float angle_step = 2 * PI / fragment;
    Vec2 p = c.m_center + Vec2::X_UNIT * c.m_radius;
    setRenderColor(color);
    for (int i = 1; i <= fragment; i++) {
        Vec2 new_p = c.m_center;
        float angle = angle_step * i;
        new_p.x += c.m_radius * std::cos(angle);
        new_p.y += c.m_radius * std::sin(angle);
        SDL_RenderLine(m_renderer, p.x, p.y, new_p.x, new_p.y);
        p = new_p;
    }
}

void Renderer::FillRect(const Rect& r, const Color& c) {
    setRenderColor(c);
    Vec2 tl = r.m_center - r.m_half_size;
    SDL_FRect rect{tl.x, tl.y, r.m_half_size.w * 2.0f, r.m_half_size.h * 2.0f};
    SDL_CALL(SDL_RenderFillRect(m_renderer, &rect));
}

void Renderer::DrawImage(const Image& image, const Region& src,
                         const Region& dst, Degrees rotation,
                         const Vec2& center, Flags<Flip> flip) {
    SDL_FRect src_rect, dst_rect;
    src_rect.x = src.m_topleft.x;
    src_rect.y = src.m_topleft.y;
    src_rect.w = src.m_size.w;
    src_rect.h = src.m_size.h;

    dst_rect.x = dst.m_topleft.x;
    dst_rect.y = dst.m_topleft.y;
    dst_rect.w = dst.m_size.w;
    dst_rect.h = dst.m_size.h;

    SDL_FPoint sdl_center;
    sdl_center.x = center.x;
    sdl_center.y = center.y;
    SDL_CALL(SDL_RenderTextureRotated(m_renderer, image.GetTexture(), &src_rect,
                                      &dst_rect, rotation.Value(), &sdl_center,
                                      static_cast<SDL_FlipMode>(flip.Value())));
}

void Renderer::DrawTiled(const Image& image, const Region& src,
                         const Region& dst, float scale) {
    SDL_FRect src_rect, dst_rect;
    src_rect.x = src.m_topleft.x;
    src_rect.y = src.m_topleft.y;
    src_rect.w = src.m_size.w;
    src_rect.h = src.m_size.h;

    dst_rect.x = dst.m_topleft.x;
    dst_rect.y = dst.m_topleft.y;
    dst_rect.w = dst.m_size.w * (scale + 0.01);
    dst_rect.h = dst.m_size.h * (scale + 0.01);

    SDL_CALL(SDL_RenderTextureTiled(m_renderer, image.GetTexture(), &src_rect,
                                    scale, &dst_rect));
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

SDL_Renderer* Renderer::GetRenderer() const {
    return m_renderer;
}

void Renderer::setRenderColor(const Color& c) {
    SDL_CALL(SDL_SetRenderDrawColor(m_renderer, c.r * 255, c.g * 255, c.b * 255,
                                    c.a * 255));
}