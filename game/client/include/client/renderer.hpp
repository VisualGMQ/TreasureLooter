#pragma once
#include <variant>

#include "SDL3/SDL.h"
#include "common/flag.hpp"
#include "common/image.hpp"
#include "common/math.hpp"
#include "schema/common.hpp"
#include "schema/flip.hpp"
#include "client/font.hpp"
#include "client/window.hpp"
#include <optional>

class Camera;
class Image;

struct DrawImageCommand {
    const ImageBase* m_image;
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
    const ImageBase* m_image;
    Image9Grid m_grid;
    Region m_src;
    Rect m_dst;
    float border_scale;
};

struct DrawImageExCommand {
    const ImageBase* m_image;
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
    float m_y_sorting{};
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
                  double z_order = 0,
                  bool use_camera = true,
                  float y_sorting = 0);

    void DrawRect(const Rect&, const Color&, double z_order = 0,
                  bool use_camera = true,
                  float y_sorting = 0);

    void DrawCircle(const Circle&, const Color&, uint32_t fragment = 20,
                    double z_order = 0,
                    bool use_camera = true,
                    float y_sorting = 0);

    void FillRect(const Rect&, const Color&, double z_order = 0,
                  bool use_camera = true,
                  float y_sorting = 0);

    void DrawImage(const ImageBase&, const Region& src, const Region& dst,
                   const Color& color,
                   Degrees rotation, const Vec2& center, Flags<Flip>,
                   double z_order = 0, bool use_camera = true,
                   float y_sorting = 0);

    void DrawImage9Grid(const ImageBase&, const Region& src, const Region& dst,
                        const Color& color,
                        const Image9Grid&, float border_scale = 1.0,
                        double z_order = 0, bool use_camera = true,
                        float y_sorting = 0);

    void DrawImageEx(const ImageBase& image, const Region& src, const Vec2& topleft,
                     const Vec2& topright, const Vec2& bottomleft,
                     const Color& color,
                     double z_order = 0, bool use_camera = true,
                     float y_sorting = 0);

    void Clear();

    void ApplyDrawcall();
    void Present();

    SDL_Renderer* GetRenderer() const;

    void BeginYSorting();
    void EndYSorting();
    bool IsRecordingYSorting() const;

private:
    SDL_Renderer* m_renderer{};
    SDL_Color m_clear_color;
    SDL_Texture* m_text_texture{};
    std::vector<DrawCommand> m_draw_commands;
    std::vector<std::pair<size_t, size_t>> m_y_sorting_range;
    bool m_is_y_sorting_range_close = true;

    void transformByCamera(const Camera&, Vec2* center, Vec2* size) const;
    void resizeTexture(const Vec2UI& new_size);
    void applyDrawCommands();
    void sortDrawCommands();
};
