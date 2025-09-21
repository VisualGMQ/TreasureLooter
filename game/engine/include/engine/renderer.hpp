#pragma once
#include "SDL3/SDL.h"
#include "flag.hpp"
#include "math.hpp"
#include "schema/common.hpp"
#include "schema/flip.hpp"
#include "text.hpp"
#include "window.hpp"

class Camera;
class Image;

struct Image9Grid {
    float left = 0;
    float right = 0;
    float top = 0;
    float bottom = 0;
    float scale = 1.0;

    bool IsValid() const noexcept {
        return !((left == 0 && right == 0) || (top == 0 && bottom == 0));
    }
};

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

    void DrawCircle(const Circle&, const Color&, uint32_t fragment = 20,
                    bool use_camera = true);

    void FillRect(const Rect&, const Color&, bool use_camera = true);

    void DrawImage(const Image&, const Region& src, const Region& dst,
                   Degrees rotation, const Vec2& center, Flags<Flip>,
                   bool use_camera = true);

    void DrawImage9Grid(const Image&, const Region& src, const Region& dst,
                        const Image9Grid&, float border_scale = 1.0, bool use_camera = true);

    void DrawRectEx(const Image& image, const Region& src, const Vec2& topleft,
                    const Vec2& topright, const Vec2& bottomleft,
                    bool use_camera = true);

    void DrawText(const std::string& text, FontHandle font,
                  const Vec2& position, const Vec2& size, const Color& color);

    void Clear();

    void Present();

    SDL_Renderer* GetRenderer() const;

private:
    SDL_Renderer* m_renderer{};
    SDL_Color m_clear_color;
    SDL_Texture* m_text_texture{};

    void setRenderColor(const Color& color);

    void transformByCamera(const Camera&, Vec2* center, Vec2* size) const;
    void resizeTexture(const Vec2UI& new_size);
};
