#pragma once
#include "pch.hpp"
#include "texture.hpp"
#include "window.hpp"
#include "flags.hpp"
#include "transform.hpp"

namespace tl {

enum class Flip {
    None = SDL_FLIP_NONE,
    Horizontal = SDL_FLIP_HORIZONTAL,
    Vertical = SDL_FLIP_VERTICAL,
};

class Renderer {
public:
    friend class Texture;
    friend class Context;

    explicit Renderer(Window& window);
    ~Renderer();

    void Clear(const Color&) const;
    void Present() const;
    void SetScale(const Vec2& scale) const;
    void DrawLine(const Vec2& p1, const Vec2& p2, const Color&) const;
    void DrawRect(const Rect&, const Color&) const;
    void DrawCircle(const Circle&, const Color&) const;
    void FillRect(const Rect&, const Color&) const;
    void DrawTexture(const Texture&, const Rect& srcRect, const Rect& dstRect,
                     float degree, const Vec2& rotCenter, Flags<Flip> flip, const Color&) const;
    void DrawTexture(const Texture&, const Rect& region, const Transform& trans,
                     const Vec2& anchor, Flags<Flip> flip,
                     const Color&) const;

    operator bool() const;

private:
    SDL_Renderer* renderer_;

    void setDrawColor(const Color&) const;
};

}  // namespace tl
