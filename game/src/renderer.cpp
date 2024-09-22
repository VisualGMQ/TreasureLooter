#include "renderer.hpp"
#include "log.hpp"
#include <SDL_render.h>

namespace tl {

Renderer::Renderer(Window& window) {
    renderer_ = SDL_CreateRenderer(window.window_, -1, 0);
    if (!renderer_) {
        LOGE("renderer create failed");
    }
}

Renderer::~Renderer() {
    SDL_DestroyRenderer(renderer_);
}

Renderer::operator bool() const {
    return renderer_;
}

void Renderer::Clear(const Color& c) {
    setDrawColor(c);
    SDL_RenderClear(renderer_);
}

void Renderer::Present() {
    SDL_RenderPresent(renderer_);
}

void Renderer::DrawLine(const Vec2& p1, const Vec2& p2, const Color& c) {
    setDrawColor(c);
    SDL_RenderDrawLineF(renderer_, p1.x, p1.y, p2.x, p2.y);
}

void Renderer::setDrawColor(const Color& c) {
    SDL_SetRenderDrawColor(renderer_, c.r, c.g, c.b, c.a);
}

void Renderer::DrawRect(const Rect& r, const Color& c) {
    setDrawColor(c);
    SDL_RenderDrawRectF(renderer_, (SDL_FRect*)&r);
}

void Renderer::FillRect(const Rect& r, const Color& c) {
    setDrawColor(c);
    SDL_RenderFillRectF(renderer_, (SDL_FRect*)&r);
}

void Renderer::DrawTexture(const Texture& texture, const Rect& srcRect,
                           const Rect& dstRect, float degree,
                           const Vec2& rotCenter, Flags<Flip> flip) {
    SDL_Rect src;
    src.x = srcRect.position.x;
    src.y = srcRect.position.y;
    src.w = srcRect.size.w;
    src.h = srcRect.size.h;
    SDL_RenderCopyExF(renderer_, texture.texture_, &src, (SDL_FRect*)&dstRect,
                      degree, (SDL_FPoint*)&rotCenter,
                      static_cast<SDL_RendererFlip>(Flip{flip}));
}

}  // namespace tl
