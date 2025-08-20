#pragma once
#include "SDL3/SDL.h"
#include "flag.hpp"
#include "math.hpp"
#include "schema/common.hpp"
#include "schema/flip.hpp"
#include "window.hpp"

class Camera;
class Image;

class Renderer {
public:
    Renderer(Window& window);
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;
    ~Renderer();

    void SetClearColor(const Color&);

    void DrawLine(const Vec2& p1, const Vec2& p2, const Color& color,
                  bool use_camera = true);
    void DrawRect(const Rect&, const Color&, bool use_camera = true);
    void DrawCircle(const Circle&, const Color&, uint32_t fragment = 20, bool use_camera = true);
    void FillRect(const Rect&, const Color&, bool use_camera = true);
    void DrawImage(const Image&, const Region& src, const Region& dst,
                   Degrees rotation, const Vec2& center, Flags<Flip>,
                   bool use_camera = true);
    void DrawRectEx(const Image& image, const Region& src, const Vec2& topleft,
                    const Vec2& topright, const Vec2& bottomleft, bool use_camera = true);

    void Clear();
    void Present();

    SDL_Renderer* GetRenderer() const;

private:
    SDL_Renderer* m_renderer{};
    SDL_Color m_clear_color;

    void setRenderColor(const Color& color);

    void transformByCamera(const Camera&, Vec2* center, Vec2* size) const;
};