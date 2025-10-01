#include "engine/renderer.hpp"

#include "engine/camera.hpp"
#include "engine/context.hpp"
#include "engine/image.hpp"
#include "engine/log.hpp"
#include "engine/profile.hpp"
#include "engine/sdl_call.hpp"

Renderer::Renderer(Window& window) {
    m_renderer = SDL_CreateRenderer(window.GetWindow(), nullptr);
    if (!m_renderer) {
        LOGE("create SDL renderer failed: {}", SDL_GetError());
    }
    SDL_CALL(SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND));
}

Renderer::~Renderer() {
    SDL_DestroyTexture(m_text_texture);
    SDL_DestroyRenderer(m_renderer);
}

void Renderer::SetClearColor(const Color& c) {
    m_clear_color.r = c.r * 255;
    m_clear_color.g = c.g * 255;
    m_clear_color.b = c.b * 255;
    m_clear_color.a = c.a * 255;
}

void Renderer::DrawLine(const Vec2& p1, const Vec2& p2, const Color& color,
                        float z_order, bool use_camera) {
    Vec2 dp1 = p1;
    Vec2 dp2 = p2;
    if (use_camera) {
        transformByCamera(CURRENT_CONTEXT.m_camera, &dp1, nullptr);
        transformByCamera(CURRENT_CONTEXT.m_camera, &dp2, nullptr);
    }

    DrawCommand cmd;
    cmd.m_color = color;
    cmd.m_z_order = z_order;

    DrawLineCommand cmd_line;
    cmd_line.m_p1 = dp1;
    cmd_line.m_p2 = dp2;

    cmd.m_cmd = cmd_line;

    m_draw_commands.emplace_back(std::move(cmd));
}

void Renderer::DrawRect(const Rect& r, const Color& c, float z_order,
                        bool use_camera) {
    Rect dst = r;
    if (use_camera) {
        transformByCamera(CURRENT_CONTEXT.m_camera, &dst.m_center,
                          &dst.m_half_size);
    }

    DrawCommand cmd;
    cmd.m_color = c;
    cmd.m_z_order = z_order;

    DrawRectCommand cmd_rect;
    cmd_rect.m_rect = dst;

    cmd.m_cmd = cmd_rect;

    m_draw_commands.emplace_back(std::move(cmd));
}

void Renderer::DrawCircle(const Circle& c, const Color& color,
                          uint32_t fragment, float z_order, bool use_camera) {
    Radians angle_step = 2 * PI / fragment;
    Vec2 p = c.m_center + Vec2::X_UNIT * c.m_radius;
    auto& camera = CURRENT_CONTEXT.m_camera;
    if (use_camera) {
        transformByCamera(camera, &p, nullptr);
    }
    for (int i = 1; i <= fragment; i++) {
        Vec2 new_p = c.m_center;
        Radians angle = angle_step * i;
        new_p.x += c.m_radius * std::cos(angle.Value());
        new_p.y += c.m_radius * std::sin(angle.Value());
        if (use_camera) {
            transformByCamera(camera, &new_p, nullptr);
        }

        DrawCommand cmd;
        cmd.m_color = color;
        cmd.m_z_order = z_order;

        DrawLineCommand cmd_line;
        cmd_line.m_p1 = p;
        cmd_line.m_p2 = new_p;
        cmd.m_cmd = cmd_line;

        m_draw_commands.emplace_back(std::move(cmd));
        p = new_p;
    }
}

void Renderer::FillRect(const Rect& r, const Color& c, float z_order,
                        bool use_camera) {
    Rect dst = r;
    if (use_camera) {
        transformByCamera(CURRENT_CONTEXT.m_camera, &dst.m_center,
                          &dst.m_half_size);
    }

    DrawCommand cmd;
    cmd.m_color = c;
    cmd.m_z_order = z_order;

    FillRectCommand cmd_rect;
    cmd_rect.m_rect = dst;
    cmd.m_cmd = cmd_rect;

    m_draw_commands.emplace_back(std::move(cmd));
}

void Renderer::DrawImage(const Image& image, const Region& src,
                         const Region& dst,
                         const Color& color,
                         Degrees rotation,
                         const Vec2& center, Flags<Flip> flip,
                         float z_order,
                         bool use_camera) {
    Rect dst_region;
    dst_region.m_half_size = dst.m_size * 0.5;
    dst_region.m_center = dst.m_topleft + dst_region.m_half_size;

    if (use_camera) {
        transformByCamera(CURRENT_CONTEXT.m_camera, &dst_region.m_center,
                          &dst_region.m_half_size);
    }

    Vec2 rot_center = center;
    if (use_camera) {
        transformByCamera(CURRENT_CONTEXT.m_camera, &rot_center, nullptr);
    }

    DrawCommand cmd;
    cmd.m_z_order = z_order;
    cmd.m_color = color;

    DrawImageCommand cmd_image;
    cmd_image.m_src = src;
    cmd_image.m_dst = dst_region;
    cmd_image.m_rot_center = rot_center;
    cmd_image.m_rotation = rotation;
    cmd_image.m_flip = flip;
    cmd_image.m_image = &image;

    cmd.m_cmd = cmd_image;

    m_draw_commands.emplace_back(std::move(cmd));
}

void Renderer::DrawImage9Grid(const Image& image, const Region& src,
                              const Region& dst,
                              const Color& color,
                              const Image9Grid& grid,
                              float border_scale, float z_order,
                              bool use_camera) {
    Rect dst_region;
    dst_region.m_half_size = dst.m_size * 0.5;
    dst_region.m_center = dst.m_topleft + dst_region.m_half_size;

    if (use_camera) {
        transformByCamera(GAME_CONTEXT.m_camera, &dst_region.m_center,
                          &dst_region.m_half_size);
    }

    DrawCommand cmd;
    cmd.m_z_order = z_order;
    cmd.m_color = color;

    DrawImage9GridCommand cmd_image;
    cmd_image.m_src = src;
    cmd_image.m_dst = dst_region;
    cmd_image.m_image = &image;
    cmd_image.border_scale = border_scale;
    cmd_image.m_grid = grid;

    cmd.m_cmd = cmd_image;

    m_draw_commands.emplace_back(std::move(cmd));
}

void Renderer::DrawImageEx(const Image& image, const Region& src,
                           const Vec2& topleft, const Vec2& topright,
                           const Vec2& bottomleft,
                           const Color& color,
                           float z_order,
                           bool use_camera) {
    Vec2 tl{topleft.x, topleft.y}, tr{topright.x, topright.y},
         bl{bottomleft.x, bottomleft.y};
    if (use_camera) {
        transformByCamera(CURRENT_CONTEXT.m_camera, &tl, nullptr);
        transformByCamera(CURRENT_CONTEXT.m_camera, &tr, nullptr);
        transformByCamera(CURRENT_CONTEXT.m_camera, &bl, nullptr);
    }

    DrawCommand cmd;
    cmd.m_z_order = z_order;
    cmd.m_color = color;

    DrawImageExCommand cmd_image;
    cmd_image.m_image = &image;
    cmd_image.m_src = src;
    cmd_image.m_origin = tl;
    cmd_image.m_down = bl;
    cmd_image.m_right = tr;

    cmd.m_cmd = cmd_image;
    m_draw_commands.emplace_back(std::move(cmd));
}

void Renderer::Clear() {
    SDL_CALL(SDL_SetRenderDrawColor(m_renderer, m_clear_color.r,
        m_clear_color.g, m_clear_color.b,
        m_clear_color.a));
    SDL_CALL(SDL_RenderClear(m_renderer));
    m_draw_commands.clear();
}

void Renderer::Present() {
    PROFILE_RENDERING_SECTION(__FUNCTION__);
    sortDrawCommands();
    applyDrawCommands();
    SDL_CALL(SDL_RenderPresent(m_renderer));
}

SDL_Renderer* Renderer::GetRenderer() const {
    return m_renderer;
}

void Renderer::transformByCamera(const Camera& camera, Vec2* center,
                                 Vec2* size) const {
    if (center) {
        auto window_size = CURRENT_CONTEXT.m_window->GetWindowSize();
        *center = (*center - camera.GetPosition()) * camera.GetScale() +
                  window_size * 0.5;
    }
    if (size) {
        *size *= camera.GetScale();
    }
}

void Renderer::resizeTexture(const Vec2UI& new_size) {
    float w, h;
    SDL_GetTextureSize(m_text_texture, &w, &h);
    if (new_size.w <= w && new_size.h <= h) {
        return;
    }

    if (m_text_texture) {
        SDL_DestroyTexture(m_text_texture);
    }
    m_text_texture =
        SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888,
                          SDL_TEXTUREACCESS_STATIC, new_size.w, new_size.h);
}

struct ApplyDrawCmdVisitor {
    ApplyDrawCmdVisitor(SDL_Renderer* renderer, const Color& color)
        : m_renderer{renderer}, m_color{color} {
    }

    void operator()(const DrawLineCommand& cmd) {
        setRenderColor(m_color);
        SDL_CALL(
            SDL_RenderLine(m_renderer, cmd.m_p1.x, cmd.m_p1.y, cmd.m_p2.x, cmd.
                m_p2.y));
    }

    void operator()(const DrawRectCommand& cmd) {
        setRenderColor(m_color);
        Vec2 tl = cmd.m_rect.m_center - cmd.m_rect.m_half_size;
        SDL_FRect rect{tl.x, tl.y, cmd.m_rect.m_half_size.w * 2.0f,
                       cmd.m_rect.m_half_size.h * 2.0f};

        SDL_CALL(SDL_RenderRect(m_renderer, &rect));
    }

    void operator()(const DrawImageCommand& cmd) {
        SDL_FRect src_rect, dst_rect;
        src_rect.x = cmd.m_src.m_topleft.x;
        src_rect.y = cmd.m_src.m_topleft.y;
        src_rect.w = cmd.m_src.m_size.w;
        src_rect.h = cmd.m_src.m_size.h;

        auto top_left = cmd.m_dst.m_center - cmd.m_dst.m_half_size;
        dst_rect.x = top_left.x;
        dst_rect.y = top_left.y;
        dst_rect.w = cmd.m_dst.m_half_size.w * 2.0f;
        dst_rect.h = cmd.m_dst.m_half_size.h * 2.0f;

        SDL_FPoint rot_center{cmd.m_rot_center.x, cmd.m_rot_center.y};

        SDL_SetTextureColorModFloat(cmd.m_image->GetTexture(), m_color.r, m_color.g, m_color.b);
        SDL_SetTextureAlphaModFloat(cmd.m_image->GetTexture(), m_color.a);
        SDL_CALL(
            SDL_RenderTextureRotated(m_renderer, cmd.m_image->GetTexture(), &
                src_rect,
                &dst_rect, cmd.m_rotation.Value(), &rot_center,
                static_cast<SDL_FlipMode>(cmd.m_flip.Value())));
    }

    void operator()(const DrawImageExCommand& cmd) {
        SDL_FRect rect = {cmd.m_src.m_topleft.x, cmd.m_src.m_topleft.y,
                          cmd.m_src.m_size.w,
                          cmd.m_src.m_size.h};
        SDL_FPoint sdl_tl{cmd.m_origin.x, cmd.m_origin.y}, sdl_tr{
                       cmd.m_right.x, cmd.m_right.y}, sdl_bl{
                       cmd.m_down.x, cmd.m_down.y};
        SDL_SetTextureColorModFloat(cmd.m_image->GetTexture(), m_color.r, m_color.g, m_color.b);
        SDL_SetTextureAlphaModFloat(cmd.m_image->GetTexture(), m_color.a);
        SDL_CALL(
            SDL_RenderTextureAffine(m_renderer, cmd.m_image->GetTexture(), &rect
                ,
                &sdl_tl, &sdl_tr, &sdl_bl));
    }

    void operator()(const FillRectCommand& cmd) {
        setRenderColor(m_color);
        Vec2 tl = cmd.m_rect.m_center - cmd.m_rect.m_half_size;
        SDL_FRect rect{tl.x, tl.y, cmd.m_rect.m_half_size.w * 2.0f,
                       cmd.m_rect.m_half_size.h * 2.0f};
        SDL_CALL(SDL_RenderFillRect(m_renderer, &rect));
    }

    void operator()(const DrawImage9GridCommand& cmd) {
        SDL_FRect final_rect;

        auto top_left = cmd.m_dst.m_center - cmd.m_dst.m_half_size;
        final_rect.x = top_left.x;
        final_rect.y = top_left.y;
        final_rect.w = cmd.m_dst.m_half_size.w * 2.0f;
        final_rect.h = cmd.m_dst.m_half_size.h * 2.0f;

        float scaled_left = cmd.m_grid.left * cmd.border_scale;
        float scaled_right = cmd.m_grid.right * cmd.border_scale;
        float scaled_top = cmd.m_grid.top * cmd.border_scale;
        float scaled_bottom = cmd.m_grid.bottom * cmd.border_scale;

        // top left corner
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x;
            src_rect.y = cmd.m_src.m_topleft.y;
            src_rect.w = cmd.m_grid.left;
            src_rect.h = cmd.m_grid.top;

            SDL_FRect dst_rect;
            dst_rect.x = top_left.x;
            dst_rect.y = top_left.y;
            dst_rect.w = scaled_left;
            dst_rect.h = scaled_top;
            SDL_CALL(
                SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(), &
                    src_rect,
                    &dst_rect));
        }

        // top border
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x + cmd.m_grid.left;
            src_rect.y = cmd.m_src.m_topleft.y;
            src_rect.w = cmd.m_src.m_size.w - cmd.m_grid.left - cmd.m_grid.
                         right;
            src_rect.h = cmd.m_grid.top;

            SDL_FRect dst_rect;
            dst_rect.x = top_left.x + scaled_left;
            dst_rect.y = top_left.y;
            dst_rect.w = final_rect.w - scaled_left - scaled_right;
            dst_rect.h = scaled_top;
            SDL_CALL(
                SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(), &
                    src_rect,
                    &dst_rect));
        }

        // top right
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x + cmd.m_src.m_size.w - cmd.m_grid
                         .right;
            src_rect.y = cmd.m_src.m_topleft.y;
            src_rect.w = cmd.m_grid.right;
            src_rect.h = cmd.m_grid.top;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x + final_rect.w - scaled_right;
            dst_rect.y = top_left.y;
            dst_rect.w = scaled_right;
            dst_rect.h = scaled_top;
            SDL_CALL(
                SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(), &
                    src_rect,
                    &dst_rect));
        }

        // middle left
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x;
            src_rect.y = cmd.m_src.m_topleft.y + cmd.m_grid.top;
            src_rect.w = cmd.m_grid.left;
            src_rect.h = cmd.m_src.m_size.h - cmd.m_grid.top - cmd.m_grid.
                         bottom;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x;
            dst_rect.y = final_rect.y + scaled_top;
            dst_rect.w = scaled_left;
            dst_rect.h = final_rect.h - scaled_top - scaled_bottom;
            SDL_CALL(
                SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(), &
                    src_rect,
                    &dst_rect));
        }

        // middle middle
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x + cmd.m_grid.left;
            src_rect.y = cmd.m_src.m_topleft.y + cmd.m_grid.top;
            src_rect.w = cmd.m_src.m_size.w - cmd.m_grid.left - cmd.m_grid.
                         right;
            src_rect.h = cmd.m_src.m_size.h - cmd.m_grid.top - cmd.m_grid.
                         bottom;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x + scaled_left;
            dst_rect.y = final_rect.y + scaled_top;
            dst_rect.w = final_rect.w - scaled_left - scaled_bottom;
            dst_rect.h = final_rect.h - scaled_top - scaled_bottom;
            SDL_CALL(
                SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(), &
                    src_rect,
                    &dst_rect));
        }

        // middle right
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x + cmd.m_src.m_size.w - cmd.m_grid
                         .right;
            src_rect.y = cmd.m_src.m_topleft.y + cmd.m_grid.top;
            src_rect.w = cmd.m_grid.right;
            src_rect.h = cmd.m_src.m_size.h - cmd.m_grid.top - cmd.m_grid.
                         bottom;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x + final_rect.w - scaled_right;
            dst_rect.y = final_rect.y + scaled_top;
            dst_rect.w = scaled_right;
            dst_rect.h = final_rect.h - scaled_top - scaled_bottom;
            SDL_CALL(
                SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(), &
                    src_rect,
                    &dst_rect));
        }

        // bottom left
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x;
            src_rect.y = cmd.m_src.m_topleft.y + cmd.m_src.m_size.h - cmd.m_grid
                         .bottom;
            src_rect.w = cmd.m_grid.right;
            src_rect.h = cmd.m_grid.bottom;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x;
            dst_rect.y = final_rect.y + final_rect.h - scaled_bottom;
            dst_rect.w = scaled_left;
            dst_rect.h = scaled_bottom;
            SDL_CALL(
                SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(), &
                    src_rect,
                    &dst_rect));
        }

        // bottom middle
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x + cmd.m_grid.left;
            src_rect.y = cmd.m_src.m_topleft.y + cmd.m_src.m_size.h - cmd.m_grid
                         .bottom;
            src_rect.w = cmd.m_src.m_topleft.x + cmd.m_src.m_size.w - cmd.m_grid
                         .right - cmd.m_grid.
                                      left;
            src_rect.h = cmd.m_grid.bottom;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x + scaled_left;
            dst_rect.y = final_rect.y + final_rect.h - scaled_bottom;
            dst_rect.w = final_rect.w - scaled_left - scaled_right;
            dst_rect.h = scaled_bottom;
            SDL_CALL(
                SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(), &
                    src_rect,
                    &dst_rect));
        }

        // bottom right
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x + cmd.m_src.m_size.w - cmd.m_grid
                         .right;
            src_rect.y = cmd.m_src.m_topleft.y + cmd.m_src.m_size.h - cmd.m_grid
                         .bottom;
            src_rect.w = cmd.m_grid.right;
            src_rect.h = cmd.m_grid.bottom;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x + final_rect.w - scaled_right;
            dst_rect.y = final_rect.y + final_rect.h - scaled_bottom;
            dst_rect.w = scaled_right;
            dst_rect.h = scaled_bottom;
            SDL_SetTextureColorModFloat(cmd.m_image->GetTexture(), m_color.r, m_color.g, m_color.b);
            SDL_SetTextureAlphaModFloat(cmd.m_image->GetTexture(), m_color.a);
            SDL_CALL(
                SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(), &
                    src_rect,
                    &dst_rect));
        }
    }

private:
    SDL_Renderer* m_renderer;
    Color m_color;

    void setRenderColor(const Color& c) {
        SDL_CALL(
            SDL_SetRenderDrawColor(m_renderer, c.r * 255, c.g * 255, c.b * 255,
                c.a * 255));
    }
};

void Renderer::applyDrawCommands() {
    PROFILE_RENDERING_SECTION(__FUNCTION__);
    for (auto& cmd : m_draw_commands) {
        ApplyDrawCmdVisitor visitor{m_renderer, cmd.m_color};
        std::visit(visitor, cmd.m_cmd);
    }
}

void Renderer::sortDrawCommands() {
    PROFILE_RENDERING_SECTION(__FUNCTION__);
    std::stable_sort(m_draw_commands.begin(), m_draw_commands.end(),
                     [](const DrawCommand& lhs, const DrawCommand& rhs) {
                         return lhs.m_z_order < rhs.m_z_order;
                     });
}

float GetZOrderByYSorting(float y, RenderLayer layer) {
    return 1.0 - 1.0 / y + static_cast<float>(layer);
}
