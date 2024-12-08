#include "renderer.hpp"
#include "log.hpp"
#include "macro.hpp"
#include "profile.hpp"

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
    drawList_.PushClearCmd(c);
}

void Renderer::SetCamera(const Camera& camera) {
    camera_ = camera;
}

void Renderer::Update() {
    PROFILE_FUNC(); 
    drawList_.SortByOrder();
    drawList_.Execute(renderer_);
    drawList_.Clear();
}

void Renderer::Present() const {
    PROFILE_FUNC(); 
    SDL_RenderPresent(renderer_);
}

void Renderer::DrawLine(const Vec2& p1, const Vec2& p2, const Color& c,
                        float order) {
    drawList_.PushLineDrawCmd(p1 - camera_.GetGlobalOffset(), p2 - camera_.GetGlobalOffset(), c,
                              order);
}

void Renderer::DrawRect(const Rect& r, const Color& c, float order) {
    drawList_.PushRectDrawCmd(r.position - camera_.GetGlobalOffset(), r.size, c, order);
}

void Renderer::FillRect(const Rect& r, const Color& c, float order) {
    drawList_.PushRectFillCmd(r.position - camera_.GetGlobalOffset(), r.size, c, order);
}

void Renderer::DrawCircle(const Circle& c, const Color& color, float order) {
    drawList_.PushCircleDrawCmd(c.center - camera_.GetGlobalOffset(), c.radius, color,
                                order);
}

void Renderer::SetScale(const Vec2& scale) const {
    SDL_RenderSetScale(renderer_, scale.w, scale.h);
}

void Renderer::DrawTexture(const Texture& texture, const Rect& region,
                           const Transform& trans, const Vec2& anchor,
                           Flags<Flip> flip, const Color& color, float order) {
    Transform transWithCamera = trans;
    transWithCamera.position -= camera_.GetGlobalOffset();

    drawList_.PushTextureDrawCmd(&texture, region, transWithCamera, anchor,
                                 flip, color, order);
}

}  // namespace tl
