#include "client/renderer.hpp"

#include <cmath>

#include "client/camera.hpp"
#include "client/context.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/physics.hpp"
#include "common/profile.hpp"
#include "common/sdl_call.hpp"

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
                        double z_order, bool use_camera, float y_sorting) {
    Vec2 dp1 = p1;
    Vec2 dp2 = p2;
    if (use_camera) {
        transformByCamera(CLIENT_CONTEXT.m_camera, &dp1, nullptr);
        transformByCamera(CLIENT_CONTEXT.m_camera, &dp2, nullptr);
    }

    DrawCommand cmd;
    cmd.m_color = color;
    cmd.m_z_order = z_order;
    cmd.m_y_sorting = y_sorting;

    DrawLineCommand cmd_line;
    cmd_line.m_p1 = dp1;
    cmd_line.m_p2 = dp2;

    cmd.m_cmd = cmd_line;

    m_draw_commands.emplace_back(std::move(cmd));
}

void Renderer::DrawRect(const Rect& r, const Color& c, double z_order,
                        bool use_camera, float y_sorting) {
    Rect dst = r;
    if (use_camera) {
        transformByCamera(CLIENT_CONTEXT.m_camera, &dst.m_center,
                          &dst.m_half_size);
    }

    DrawCommand cmd;
    cmd.m_color = c;
    cmd.m_z_order = z_order;
    cmd.m_y_sorting = y_sorting;

    DrawRectCommand cmd_rect;
    cmd_rect.m_rect = dst;

    cmd.m_cmd = cmd_rect;

    m_draw_commands.emplace_back(std::move(cmd));
}

void Renderer::DrawCircle(const Circle& c, const Color& color,
                          uint32_t fragment, double z_order, bool use_camera,
                          float y_sorting) {
    Radians angle_step = 2 * PI / fragment;
    Vec2 p = c.m_center + Vec2::X_UNIT * c.m_radius;
    auto& camera = CLIENT_CONTEXT.m_camera;
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
        cmd.m_y_sorting = y_sorting;

        DrawLineCommand cmd_line;
        cmd_line.m_p1 = p;
        cmd_line.m_p2 = new_p;
        cmd.m_cmd = cmd_line;

        m_draw_commands.emplace_back(std::move(cmd));
        p = new_p;
    }
}

void Renderer::FillRect(const Rect& r, const Color& c, double z_order,
                        bool use_camera, float y_sorting) {
    Rect dst = r;
    if (use_camera) {
        transformByCamera(CLIENT_CONTEXT.m_camera, &dst.m_center,
                          &dst.m_half_size);
    }

    DrawCommand cmd;
    cmd.m_color = c;
    cmd.m_z_order = z_order;
    cmd.m_y_sorting = y_sorting;

    FillRectCommand cmd_rect;
    cmd_rect.m_rect = dst;
    cmd.m_cmd = cmd_rect;

    m_draw_commands.emplace_back(std::move(cmd));
}

void Renderer::DrawImage(const ImageBase& image, const Region& src,
                         const Region& dst, const Color& color,
                         Degrees rotation, const Vec2& center, Flags<Flip> flip,
                         double z_order, bool use_camera, float y_sorting) {
    Rect dst_region;
    dst_region.m_half_size = dst.m_size * 0.5;
    dst_region.m_center = dst.m_topleft + dst_region.m_half_size;

    if (use_camera) {
        transformByCamera(CLIENT_CONTEXT.m_camera, &dst_region.m_center,
                          &dst_region.m_half_size);
    }

    Vec2 rot_center = center;
    if (use_camera) {
        transformByCamera(CLIENT_CONTEXT.m_camera, &rot_center, nullptr);
    }

    DrawCommand cmd;
    cmd.m_z_order = z_order;
    cmd.m_y_sorting = y_sorting;
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

void Renderer::DrawImage9Grid(const ImageBase& image, const Region& src,
                              const Region& dst, const Color& color,
                              const Image9Grid& grid, float border_scale,
                              double z_order, bool use_camera,
                              float y_sorting) {
    Rect dst_region;
    dst_region.m_half_size = dst.m_size * 0.5;
    dst_region.m_center = dst.m_topleft + dst_region.m_half_size;

    if (use_camera) {
        transformByCamera(CLIENT_CONTEXT.m_camera, &dst_region.m_center,
                          &dst_region.m_half_size);
    }

    DrawCommand cmd;
    cmd.m_z_order = z_order;
    cmd.m_color = color;
    cmd.m_y_sorting = y_sorting;

    DrawImage9GridCommand cmd_image;
    cmd_image.m_src = src;
    cmd_image.m_dst = dst_region;
    cmd_image.m_image = &image;
    cmd_image.border_scale = border_scale;
    cmd_image.m_grid = grid;

    cmd.m_cmd = cmd_image;

    m_draw_commands.emplace_back(std::move(cmd));
}

void Renderer::DrawImageEx(const ImageBase& image, const Region& src,
                           const Vec2& topleft, const Vec2& topright,
                           const Vec2& bottomleft, const Color& color,
                           double z_order, bool use_camera, float y_sorting) {
    Vec2 tl{topleft.x, topleft.y}, tr{topright.x, topright.y},
        bl{bottomleft.x, bottomleft.y};
    if (use_camera) {
        CLIENT_CONTEXT.m_camera.transform(&tl, nullptr);
        CLIENT_CONTEXT.m_camera.transform(&tr, nullptr);
        CLIENT_CONTEXT.m_camera.transform(&bl, nullptr);
    }

    DrawCommand cmd;
    cmd.m_z_order = z_order;
    cmd.m_y_sorting = y_sorting;
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
}

void Renderer::ApplyDrawcall() {
    PROFILE_SECTION();

    sortDrawCommands();
    applyDrawCommands();

    m_draw_commands.clear();
    m_y_sorting_range.clear();
    m_is_y_sorting_range_close = true;
}

void Renderer::Present() {
    PROFILE_SECTION();

    SDL_CALL(SDL_RenderPresent(m_renderer));
}

SDL_Renderer* Renderer::GetRenderer() const {
    return m_renderer;
}

void Renderer::BeginYSorting() {
    TL_RETURN_IF_FALSE(m_is_y_sorting_range_close);

    m_is_y_sorting_range_close = false;
    m_y_sorting_range.emplace_back(
        std::make_pair<size_t, size_t>(m_draw_commands.size(), 0));
}

void Renderer::EndYSorting() {
    TL_RETURN_IF_TRUE(m_y_sorting_range.empty());

    m_is_y_sorting_range_close = true;

    auto& range = m_y_sorting_range.back();
    range.second = m_draw_commands.size();
}

bool Renderer::IsRecordingYSorting() const {
    return !m_is_y_sorting_range_close;
}

void Renderer::transformByCamera(const Camera& camera, Vec2* center,
                                 Vec2* size) const {
    if (center) {
        Vec2 window_size =
            static_cast<Vec2>(CLIENT_CONTEXT.m_window->GetWindowSize());
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
    ApplyDrawCmdVisitor(SDL_Renderer* renderer, const Vec2UI window_size)
        : m_renderer{renderer}, m_window_rect{Vec2::ZERO, Vec2{window_size}} {}

    void ChangeColor(const Color& color) { m_color = color; }

    void operator()(const DrawLineCommand& cmd) {
        setRenderColor(m_color);
        SDL_CALL(SDL_RenderLine(m_renderer, cmd.m_p1.x, cmd.m_p1.y, cmd.m_p2.x,
                                cmd.m_p2.y));
    }

    void operator()(const DrawRectCommand& cmd) {
        TL_RETURN_IF_FALSE(IsRectsIntersect(m_window_rect, cmd.m_rect));

        setRenderColor(m_color);
        Vec2 tl = cmd.m_rect.m_center - cmd.m_rect.m_half_size;
        SDL_FRect rect{tl.x, tl.y, cmd.m_rect.m_half_size.w * 2.0f,
                       cmd.m_rect.m_half_size.h * 2.0f};

        SDL_CALL(SDL_RenderRect(m_renderer, &rect));
    }

    void operator()(const DrawImageCommand& cmd) {
        TL_RETURN_IF_FALSE(IsRectsIntersect(
            m_window_rect,
            GetDrawImageAABB(cmd.m_dst, cmd.m_rotation, cmd.m_rot_center)));

        SDL_FRect src_rect, dst_rect;
        src_rect.x = cmd.m_src.m_topleft.x;
        src_rect.y = cmd.m_src.m_topleft.y;
        src_rect.w = cmd.m_src.m_size.w;
        src_rect.h = cmd.m_src.m_size.h;

        auto top_left = cmd.m_dst.m_center - cmd.m_dst.m_half_size;
        dst_rect.x = std::roundf(top_left.x);
        dst_rect.y = std::roundf(top_left.y);
        dst_rect.w = std::roundf(cmd.m_dst.m_half_size.w * 2.0f);
        dst_rect.h = std::roundf(cmd.m_dst.m_half_size.h * 2.0f);

        SDL_FPoint rot_center{cmd.m_rot_center.x, cmd.m_rot_center.y};

        SDL_SetTextureColorModFloat(cmd.m_image->GetTexture(), m_color.r,
                                    m_color.g, m_color.b);
        SDL_SetTextureAlphaModFloat(cmd.m_image->GetTexture(), m_color.a);
        SDL_CALL(SDL_RenderTextureRotated(
            m_renderer, cmd.m_image->GetTexture(), &src_rect, &dst_rect,
            cmd.m_rotation.Value(), &rot_center,
            static_cast<SDL_FlipMode>(cmd.m_flip.Value())));
    }

    void operator()(const DrawImageExCommand& cmd) {
        Vec2 br = {cmd.m_right.x + cmd.m_down.x - cmd.m_origin.x,
                   cmd.m_right.y + cmd.m_down.y - cmd.m_origin.y};
        float min_x =
            std::min({cmd.m_origin.x, cmd.m_right.x, cmd.m_down.x, br.x});
        float max_x =
            std::max({cmd.m_origin.x, cmd.m_right.x, cmd.m_down.x, br.x});
        float min_y =
            std::min({cmd.m_origin.y, cmd.m_right.y, cmd.m_down.y, br.y});
        float max_y =
            std::max({cmd.m_origin.y, cmd.m_right.y, cmd.m_down.y, br.y});
        Rect aabb{
            {(min_x + max_x) * 0.5f, (min_y + max_y) * 0.5f},
            {(max_x - min_x) * 0.5f, (max_y - min_y) * 0.5f}
        };
        TL_RETURN_IF_FALSE(IsRectsIntersect(m_window_rect, aabb));

        SDL_FRect rect = {std::roundf(cmd.m_src.m_topleft.x),
                          std::roundf(cmd.m_src.m_topleft.y),
                          std::roundf(cmd.m_src.m_size.w),
                          std::roundf(cmd.m_src.m_size.h)};
        SDL_FPoint sdl_tl{std::roundf(cmd.m_origin.x), std::roundf(cmd.m_origin.y)},
            sdl_tr{std::roundf(cmd.m_right.x), std::roundf(cmd.m_right.y)},
            sdl_bl{std::roundf(cmd.m_down.x), std::roundf(cmd.m_down.y)};
        SDL_SetTextureColorModFloat(cmd.m_image->GetTexture(), m_color.r,
                                    m_color.g, m_color.b);
        SDL_SetTextureAlphaModFloat(cmd.m_image->GetTexture(), m_color.a);
        SDL_CALL(SDL_RenderTextureAffine(m_renderer, cmd.m_image->GetTexture(),
                                         &rect, &sdl_tl, &sdl_tr, &sdl_bl));
    }

    void operator()(const FillRectCommand& cmd) {
        TL_RETURN_IF_FALSE(IsRectsIntersect(m_window_rect, cmd.m_rect));

        setRenderColor(m_color);
        Vec2 tl = cmd.m_rect.m_center - cmd.m_rect.m_half_size;
        SDL_FRect rect{tl.x, tl.y, cmd.m_rect.m_half_size.w * 2.0f,
                       cmd.m_rect.m_half_size.h * 2.0f};
        SDL_CALL(SDL_RenderFillRect(m_renderer, &rect));
    }

    void operator()(const DrawImage9GridCommand& cmd) {
        TL_RETURN_IF_FALSE(IsRectsIntersect(m_window_rect, cmd.m_dst));

        SDL_FRect final_rect;

        auto top_left = cmd.m_dst.m_center - cmd.m_dst.m_half_size;
        final_rect.x = top_left.x;
        final_rect.y = top_left.y;
        final_rect.w = cmd.m_dst.m_half_size.w * 2.0f;
        final_rect.h = cmd.m_dst.m_half_size.h * 2.0f;

        float scaled_left = cmd.m_grid.m_left * cmd.border_scale;
        float scaled_right = cmd.m_grid.m_right * cmd.border_scale;
        float scaled_top = cmd.m_grid.m_top * cmd.border_scale;
        float scaled_bottom = cmd.m_grid.m_bottom * cmd.border_scale;

        // top left corner
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x;
            src_rect.y = cmd.m_src.m_topleft.y;
            src_rect.w = cmd.m_grid.m_left;
            src_rect.h = cmd.m_grid.m_top;

            SDL_FRect dst_rect;
            dst_rect.x = top_left.x;
            dst_rect.y = top_left.y;
            dst_rect.w = scaled_left;
            dst_rect.h = scaled_top;
            SDL_CALL(SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(),
                                       &src_rect, &dst_rect));
        }

        // top border
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x + cmd.m_grid.m_left;
            src_rect.y = cmd.m_src.m_topleft.y;
            src_rect.w =
                cmd.m_src.m_size.w - cmd.m_grid.m_left - cmd.m_grid.m_right;
            src_rect.h = cmd.m_grid.m_top;

            SDL_FRect dst_rect;
            dst_rect.x = top_left.x + scaled_left;
            dst_rect.y = top_left.y;
            dst_rect.w = final_rect.w - scaled_left - scaled_right;
            dst_rect.h = scaled_top;
            SDL_CALL(SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(),
                                       &src_rect, &dst_rect));
        }

        // top right
        {
            SDL_FRect src_rect;
            src_rect.x =
                cmd.m_src.m_topleft.x + cmd.m_src.m_size.w - cmd.m_grid.m_right;
            src_rect.y = cmd.m_src.m_topleft.y;
            src_rect.w = cmd.m_grid.m_right;
            src_rect.h = cmd.m_grid.m_top;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x + final_rect.w - scaled_right;
            dst_rect.y = top_left.y;
            dst_rect.w = scaled_right;
            dst_rect.h = scaled_top;
            SDL_CALL(SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(),
                                       &src_rect, &dst_rect));
        }

        // middle left
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x;
            src_rect.y = cmd.m_src.m_topleft.y + cmd.m_grid.m_top;
            src_rect.w = cmd.m_grid.m_left;
            src_rect.h =
                cmd.m_src.m_size.h - cmd.m_grid.m_top - cmd.m_grid.m_bottom;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x;
            dst_rect.y = final_rect.y + scaled_top;
            dst_rect.w = scaled_left;
            dst_rect.h = final_rect.h - scaled_top - scaled_bottom;
            SDL_CALL(SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(),
                                       &src_rect, &dst_rect));
        }

        // middle middle
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x + cmd.m_grid.m_left;
            src_rect.y = cmd.m_src.m_topleft.y + cmd.m_grid.m_top;
            src_rect.w =
                cmd.m_src.m_size.w - cmd.m_grid.m_left - cmd.m_grid.m_right;
            src_rect.h =
                cmd.m_src.m_size.h - cmd.m_grid.m_top - cmd.m_grid.m_bottom;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x + scaled_left;
            dst_rect.y = final_rect.y + scaled_top;
            dst_rect.w = final_rect.w - scaled_left - scaled_bottom;
            dst_rect.h = final_rect.h - scaled_top - scaled_bottom;
            SDL_CALL(SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(),
                                       &src_rect, &dst_rect));
        }

        // middle right
        {
            SDL_FRect src_rect;
            src_rect.x =
                cmd.m_src.m_topleft.x + cmd.m_src.m_size.w - cmd.m_grid.m_right;
            src_rect.y = cmd.m_src.m_topleft.y + cmd.m_grid.m_top;
            src_rect.w = cmd.m_grid.m_right;
            src_rect.h =
                cmd.m_src.m_size.h - cmd.m_grid.m_top - cmd.m_grid.m_bottom;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x + final_rect.w - scaled_right;
            dst_rect.y = final_rect.y + scaled_top;
            dst_rect.w = scaled_right;
            dst_rect.h = final_rect.h - scaled_top - scaled_bottom;
            SDL_CALL(SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(),
                                       &src_rect, &dst_rect));
        }

        // bottom left
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x;
            src_rect.y = cmd.m_src.m_topleft.y + cmd.m_src.m_size.h -
                         cmd.m_grid.m_bottom;
            src_rect.w = cmd.m_grid.m_right;
            src_rect.h = cmd.m_grid.m_bottom;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x;
            dst_rect.y = final_rect.y + final_rect.h - scaled_bottom;
            dst_rect.w = scaled_left;
            dst_rect.h = scaled_bottom;
            SDL_CALL(SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(),
                                       &src_rect, &dst_rect));
        }

        // bottom middle
        {
            SDL_FRect src_rect;
            src_rect.x = cmd.m_src.m_topleft.x + cmd.m_grid.m_left;
            src_rect.y = cmd.m_src.m_topleft.y + cmd.m_src.m_size.h -
                         cmd.m_grid.m_bottom;
            src_rect.w = cmd.m_src.m_topleft.x + cmd.m_src.m_size.w -
                         cmd.m_grid.m_right - cmd.m_grid.m_left;
            src_rect.h = cmd.m_grid.m_bottom;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x + scaled_left;
            dst_rect.y = final_rect.y + final_rect.h - scaled_bottom;
            dst_rect.w = final_rect.w - scaled_left - scaled_right;
            dst_rect.h = scaled_bottom;
            SDL_CALL(SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(),
                                       &src_rect, &dst_rect));
        }

        // bottom right
        {
            SDL_FRect src_rect;
            src_rect.x =
                cmd.m_src.m_topleft.x + cmd.m_src.m_size.w - cmd.m_grid.m_right;
            src_rect.y = cmd.m_src.m_topleft.y + cmd.m_src.m_size.h -
                         cmd.m_grid.m_bottom;
            src_rect.w = cmd.m_grid.m_right;
            src_rect.h = cmd.m_grid.m_bottom;

            SDL_FRect dst_rect;
            dst_rect.x = final_rect.x + final_rect.w - scaled_right;
            dst_rect.y = final_rect.y + final_rect.h - scaled_bottom;
            dst_rect.w = scaled_right;
            dst_rect.h = scaled_bottom;
            SDL_SetTextureColorModFloat(cmd.m_image->GetTexture(), m_color.r,
                                        m_color.g, m_color.b);
            SDL_SetTextureAlphaModFloat(cmd.m_image->GetTexture(), m_color.a);
            SDL_CALL(SDL_RenderTexture(m_renderer, cmd.m_image->GetTexture(),
                                       &src_rect, &dst_rect));
        }
    }

private:
    SDL_Renderer* m_renderer;
    Rect m_window_rect;
    Color m_color;

    Rect GetDrawImageAABB(const Rect& rect, Degrees rotation,
                          const Vec2& pivot) const {
        if (rotation.Value() == 0.0f) {
            return rect;
        }
        Radians rad = rotation;
        float s = std::sin(rad.Value()), c_val = std::cos(rad.Value());
        Vec2 corners[4] = {
            {rect.m_center.x - rect.m_half_size.w,
             rect.m_center.y - rect.m_half_size.h},
            {rect.m_center.x + rect.m_half_size.w,
             rect.m_center.y - rect.m_half_size.h},
            {rect.m_center.x - rect.m_half_size.w,
             rect.m_center.y + rect.m_half_size.h},
            {rect.m_center.x + rect.m_half_size.w,
             rect.m_center.y + rect.m_half_size.h}
        };
        for (auto& c : corners) {
            float dx = c.x - pivot.x;
            float dy = c.y - pivot.y;
            c.x = pivot.x + dx * c_val - dy * s;
            c.y = pivot.y + dx * s + dy * c_val;
        }
        float min_x =
            std::min({corners[0].x, corners[1].x, corners[2].x, corners[3].x});
        float max_x =
            std::max({corners[0].x, corners[1].x, corners[2].x, corners[3].x});
        float min_y =
            std::min({corners[0].y, corners[1].y, corners[2].y, corners[3].y});
        float max_y =
            std::max({corners[0].y, corners[1].y, corners[2].y, corners[3].y});
        return {
            {(min_x + max_x) * 0.5f, (min_y + max_y) * 0.5f},
            {(max_x - min_x) * 0.5f, (max_y - min_y) * 0.5f}
        };
    }

    void setRenderColor(const Color& c) {
        SDL_CALL(SDL_SetRenderDrawColor(m_renderer, c.r * 255, c.g * 255,
                                        c.b * 255, c.a * 255));
    }
};

void Renderer::applyDrawCommands() {
    PROFILE_SECTION();

    auto window_size = CLIENT_CONTEXT.m_window->GetWindowSize();

    ApplyDrawCmdVisitor visitor{m_renderer, window_size};
    for (auto& cmd : m_draw_commands) {
        visitor.ChangeColor(cmd.m_color);
        std::visit(visitor, cmd.m_cmd);
    }
}

void Renderer::sortDrawCommands() {
    PROFILE_SECTION();

    for (auto& range : m_y_sorting_range) {
        TL_CONTINUE_IF_FALSE(range.first < range.second);

        auto it = m_draw_commands.begin();
        std::sort(it + range.first, it + range.second,
                  [](const DrawCommand& lhs, const DrawCommand& rhs) {
                      return lhs.m_y_sorting < rhs.m_y_sorting;
                  });
    }

    std::stable_sort(m_draw_commands.begin(), m_draw_commands.end(),
                     [](const DrawCommand& lhs, const DrawCommand& rhs) {
                         return lhs.m_z_order < rhs.m_z_order;
                     });
}
