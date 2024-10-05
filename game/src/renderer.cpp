#include "renderer.hpp"
#include "log.hpp"
#include "macro.hpp"

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

void Renderer::SetScale(const Vec2& scale) {
    SDL_RenderSetScale(renderer_, scale.w, scale.h);
}

void Renderer::DrawTexture(const Texture& texture, const Rect& region, const Transform& trans,
                     const Vec2& anchor, Flags<Flip> flip) {
    TL_RETURN_IF(texture);

    Rect dstRect;
    Vec2 unsignedScale = trans.scale;
    unsignedScale.x = std::abs(trans.scale.x);
    unsignedScale.y = std::abs(trans.scale.y);
    dstRect.size = unsignedScale * region.size;

    Vec2 xAxis = Rotate(Vec2::X_AXIS, trans.rotation);
    Vec2 yAxis = Rotate(Vec2::Y_AXIS, trans.rotation);
    int xSign = Sign(trans.scale.x);
    int ySign = Sign(trans.scale.y);
    Vec2 xOffset, yOffset;
    if (xSign > 0) {
        xOffset = -dstRect.size.w * anchor.x * xAxis;
    } else if (xSign < 0) {
        xOffset = -dstRect.size.w * (1.0 - anchor.x) * xAxis;
    }
    if (ySign > 0) {
        yOffset = -dstRect.size.h * anchor.y * yAxis;
    } else if (ySign < 0) {
        yOffset = -dstRect.size.h * (1.0 - anchor.y) * yAxis;
    }
    dstRect.position = trans.position + xOffset + yOffset;

    Flags<Flip> f = Flip::None;

    Vec2 flipStatus{flip & Flip::Horizontal ? -1.0f : 1.0f,
                    flip & Flip::Vertical ? -1.0f : 1.0f};

    if (trans.scale.x * flipStatus.x < 0) {
        f |= Flip::Horizontal;
    }
    if (trans.scale.y * flipStatus.y < 0) {
        f |= Flip::Vertical;
    }

    DrawTexture(texture, region, dstRect,
        trans.rotation, Vec2::ZERO, f);
}

}  // namespace tl
