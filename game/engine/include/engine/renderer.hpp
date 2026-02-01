#pragma once
#include <variant>

#include "SDL3/SDL.h"
#include "flag.hpp"
#include "image.hpp"
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

struct DrawImageCommand {
    const Image* m_image;
    Region m_src;
    Rect m_dst;
    Degrees m_rotation;
    Vec2 m_rot_center;
    Flags<Flip> m_flip;
};

struct DrawRectCommand {
    Rect m_rect{};
};

struct FillRectCommand {
    Rect m_rect{};
};

struct DrawLineCommand {
    Vec2 m_p1, m_p2;
};

struct DrawImage9GridCommand {
    const Image* m_image;
    Image9Grid m_grid;
    Region m_src;
    Rect m_dst;
    float border_scale;
};

struct DrawImageExCommand {
    const Image* m_image;
    Region m_src;
    Vec2 m_origin;
    Vec2 m_right;
    Vec2 m_down;
};

struct DrawCommand {
    std::variant<DrawRectCommand, DrawImageCommand,
                 DrawImageExCommand,
                 FillRectCommand,
                 DrawImage9GridCommand,
                 DrawLineCommand
    > m_cmd;
    float m_z_order{};
    Color m_color = Color::White;
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
                  float z_order = 0,
                  bool use_camera = true);

    void DrawRect(const Rect&, const Color&, float z_order = 0,
                  bool use_camera = true);

    void DrawCircle(const Circle&, const Color&, uint32_t fragment = 20,
                    float z_order = 0,
                    bool use_camera = true);

    void FillRect(const Rect&, const Color&, float z_order = 0,
                  bool use_camera = true);

    void DrawImage(const Image&, const Region& src, const Region& dst,
                   const Color& color,
                   Degrees rotation, const Vec2& center, Flags<Flip>,
                   float z_order = 0, bool use_camera = true);

    void DrawImage9Grid(const Image&, const Region& src, const Region& dst,
                        const Color& color,
                        const Image9Grid&, float border_scale = 1.0,
                        float z_order = 0, bool use_camera = true);

    void DrawImageEx(const Image& image, const Region& src, const Vec2& topleft,
                     const Vec2& topright, const Vec2& bottomleft,
                     const Color& color,
                     float z_order = 0, bool use_camera = true);

    void Clear();

    void Present();

    SDL_Renderer* GetRenderer() const;

private:
    SDL_Renderer* m_renderer{};
    SDL_Color m_clear_color;
    SDL_Texture* m_text_texture{};
    std::vector<DrawCommand> m_draw_commands;

    void transformByCamera(const Camera&, Vec2* center, Vec2* size) const;
    void resizeTexture(const Vec2UI& new_size);
    void applyDrawCommands();
    void sortDrawCommands();
};

float GetZOrderByYSorting(float y, RenderLayer);
