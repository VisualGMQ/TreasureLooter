#pragma once
#include "pch.hpp"
#include "texture.hpp"
#include "window.hpp"
#include "flags.hpp"

namespace tl {

enum class Flip {
    None = SDL_FLIP_NONE,
    Horizontal = SDL_FLIP_HORIZONTAL,
    Vertical = SDL_FLIP_VERTICAL,
};

class Renderer {
public:
    friend class Texture;

    explicit Renderer(Window& window);
    ~Renderer();

    void Clear(const Color&);
    void Present();
    void DrawLine(const Vec2& p1, const Vec2& p2, const Color&);
    void DrawRect(const Rect&, const Color&);
    void FillRect(const Rect&, const Color&);
    void DrawTexture(const Texture&, const Rect& srcRect, const Rect& dstRect,
                     float degree, const Vec2& rotCenter, Flags<Flip> flip = Flip::None);

    operator bool() const;

private:
    SDL_Renderer* renderer_;

    void setDrawColor(const Color&);
};

}  // namespace tl
