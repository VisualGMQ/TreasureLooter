#pragma once
#include "texture.hpp"
#include "transform.hpp"
#include "common.hpp"
#include "flags.hpp"

namespace tl {

class DrawList {
public:
    void SortByOrder();
    void Clear();
    void Execute(SDL_Renderer*) const;
    void PushClearCmd(const Color& color);
    void PushTextureDrawCmd(const Texture*, const Rect& region,
                            const Transform& trans, const Vec2& anchor,
                            Flags<Flip> flip, const Color&, float order);

    void PushLineDrawCmd(const Vec2& p1, const Vec2& p2, const Color& color,
                         float order);
    void PushLineStripDrawCmd(const Vec2* pts, uint32_t count,
                              const Color& color, float order);
    void PushLineLoopDrawCmd(const Vec2* pts, uint32_t count,
                             const Color& color, float order);
    void PushRectDrawCmd(const Vec2& topleft, const Vec2& size,
                         const Color& color, float order);
    void PushRectFillCmd(const Vec2& topleft, const Vec2& size,
                         const Color& color, float order);
    void PushCircleDrawCmd(const Vec2& center, float radius, const Color& color,
                           float order);

private:
    struct TextureDrawCmd {
        const Texture* texture = nullptr;
        Rect region;
        Color color;
        Flags<Flip> flip;
    };

    struct GeomDrawCmd {
        enum class Type {
            Unknown,
            Line,       // p1.x, p1.y, p2.x, p2.y
            LineStrip,  // p1.x, p1.y, p2.x, p2.y, ...
            LineLoop,   // p1.x, p1.y, p2.x, p2.y, ...
            Rect,       // x, y, w, h
            Circle,     // center.x, center.y, radius
        } type = Type::Unknown;
        uint32_t elemCount = 0;
        bool fill = false;
        Color color;
    };

    struct ClearCmd {
        Color color;
    };

    struct DrawCmd {
        enum class Type {
            Clear,
            Texture,
            Geom,
        } type;
        size_t beginIdx = 0;
        float order = 0;

        explicit DrawCmd(Type type) : type{type} {}

        union {
            GeomDrawCmd geom;
            TextureDrawCmd texture;
            ClearCmd clear;
        };
    };

    std::vector<DrawCmd> drawCmds_;
    std::vector<float> drawDatas_;

    void setDrawColor(SDL_Renderer*, const Color&) const;
    void executeGeomCmd(SDL_Renderer*, const GeomDrawCmd&,
                        size_t startDataIdx) const;
    void executeTextureCmd(SDL_Renderer*, const TextureDrawCmd&,
                           size_t startDataIdx) const;
};

}  // namespace tl