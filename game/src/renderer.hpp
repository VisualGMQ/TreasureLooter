#pragma once
#include "pch.hpp"
#include "texture.hpp"
#include "window.hpp"
#include "flags.hpp"
#include "transform.hpp"
#include "draw_list.hpp"

namespace tl {

class Renderer {
public:
    friend class Texture;
    friend class Context;

    explicit Renderer(Window& window);
    ~Renderer();

    void Clear(const Color&);
    void SetCamera(const Camera& camera);
    void Update();
    void Present() const;
    void SetScale(const Vec2& scale) const;
    void DrawLine(const Vec2& p1, const Vec2& p2, const Color&, float order = 0) ;
    void DrawRect(const Rect&, const Color&, float order = 0);
    void DrawCircle(const Circle&, const Color&, float order = 0);
    void FillRect(const Rect&, const Color&, float order = 0);
    void DrawTexture(const Texture&, const Rect& region, const Transform& trans,
                     const Vec2& anchor, Flags<Flip> flip,
                     const Color&, float order = 0);

    operator bool() const;

private:
    SDL_Renderer* renderer_;
    DrawList drawList_;
    Camera camera_;
};

}  // namespace tl
