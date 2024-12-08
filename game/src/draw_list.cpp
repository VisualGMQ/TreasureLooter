#include "draw_list.hpp"

#include "common.hpp"
#include "context.hpp"
#include "flags.hpp"
#include "log.hpp"
#include "macro.hpp"

namespace tl {

void DrawList::SortByOrder() {
    std::stable_sort(drawCmds_.begin(), drawCmds_.end(),
              [](const DrawCmd& lhs, const DrawCmd& rhs) {
                  return lhs.order < rhs.order;
              });
}

void DrawList::Clear() {
    drawCmds_.clear();
    drawDatas_.clear();
}

void DrawList::PushClearCmd(const Color& color) {
    DrawCmd cmd{DrawCmd::Type::Clear};
    cmd.clear.color = color;

    drawCmds_.push_back(cmd);
}

void DrawList::PushTextureDrawCmd(const Texture* texture, const Rect& region,
                                  const Transform& trans, const Vec2& anchor,
                                  Flags<Flip> flip, const Color& color,
                                  float order) {
    DrawCmd cmd{DrawCmd::Type::Texture};
    cmd.texture.texture = texture;
    cmd.texture.region = region;
    cmd.texture.color = color;
    cmd.texture.flip = flip;
    cmd.order = order;
    cmd.beginIdx = drawDatas_.size();

    drawCmds_.push_back(cmd);
    drawDatas_.push_back(trans.position.x);
    drawDatas_.push_back(trans.position.y);
    drawDatas_.push_back(trans.scale.x);
    drawDatas_.push_back(trans.scale.y);
    drawDatas_.push_back(trans.rotation);
    drawDatas_.push_back(anchor.x);
    drawDatas_.push_back(anchor.y);
}

void DrawList::PushLineDrawCmd(const Vec2& p1, const Vec2& p2,
                               const Color& color, float order) {
    DrawCmd cmd{DrawCmd::Type::Geom};
    cmd.geom.type = GeomDrawCmd::Type::Line;
    cmd.geom.elemCount = 1;
    cmd.geom.color = color;
    cmd.order = order;
    cmd.beginIdx = drawDatas_.size();

    drawCmds_.push_back(cmd);
    drawDatas_.push_back(p1.x);
    drawDatas_.push_back(p1.y);
    drawDatas_.push_back(p2.x);
    drawDatas_.push_back(p2.y);
}

void DrawList::PushLineStripDrawCmd(const Vec2* pts, uint32_t count,
                                    const Color& color, float order) {
    DrawCmd cmd{DrawCmd::Type::Geom};
    cmd.geom.type = GeomDrawCmd::Type::LineStrip;
    cmd.geom.elemCount = count;
    cmd.geom.color = color;
    cmd.order = order;
    cmd.beginIdx = drawDatas_.size();

    drawCmds_.push_back(cmd);
    for (uint32_t i = 0; i < count; i++) {
        drawDatas_.push_back(pts[i].x);
        drawDatas_.push_back(pts[i].y);
    }
}

void DrawList::PushLineLoopDrawCmd(const Vec2* pts, uint32_t count,
                                   const Color& color, float order) {
    DrawCmd cmd{DrawCmd::Type::Geom};
    cmd.geom.type = GeomDrawCmd::Type::LineLoop;
    cmd.geom.elemCount = count;
    cmd.geom.color = color;
    cmd.order = order;
    cmd.beginIdx = drawDatas_.size();

    drawCmds_.push_back(cmd);
    for (uint32_t i = 0; i < count; i++) {
        drawDatas_.push_back(pts[i].x);
        drawDatas_.push_back(pts[i].y);
    }
}

void DrawList::PushRectDrawCmd(const Vec2& topleft, const Vec2& size,
                               const Color& color, float order) {
    DrawCmd cmd{DrawCmd::Type::Geom};
    cmd.geom.type = GeomDrawCmd::Type::Rect;
    cmd.geom.elemCount = 1;
    cmd.geom.color = color;
    cmd.geom.fill = false;
    cmd.order = order;
    cmd.beginIdx = drawDatas_.size();

    drawCmds_.push_back(cmd);
    drawDatas_.push_back(topleft.x);
    drawDatas_.push_back(topleft.y);
    drawDatas_.push_back(size.w);
    drawDatas_.push_back(size.h);
}

void DrawList::PushRectFillCmd(const Vec2& topleft, const Vec2& size,
                               const Color& color, float order) {
    DrawCmd cmd{DrawCmd::Type::Geom};
    cmd.geom.type = GeomDrawCmd::Type::Rect;
    cmd.geom.elemCount = 1;
    cmd.geom.color = color;
    cmd.geom.fill = true;
    cmd.order = order;
    cmd.beginIdx = drawDatas_.size();

    drawCmds_.push_back(cmd);
    drawDatas_.push_back(topleft.x);
    drawDatas_.push_back(topleft.y);
    drawDatas_.push_back(size.w);
    drawDatas_.push_back(size.h);
}

void DrawList::PushCircleDrawCmd(const Vec2& center, float radius,
                                 const Color& color, float order) {
    DrawCmd cmd{DrawCmd::Type::Geom};
    cmd.geom.type = GeomDrawCmd::Type::Circle;
    cmd.geom.elemCount = 1;
    cmd.geom.color = color;
    cmd.order = order;
    cmd.beginIdx = drawDatas_.size();

    drawCmds_.push_back(cmd);
    drawDatas_.push_back(center.x);
    drawDatas_.push_back(center.y);
    drawDatas_.push_back(radius);
}

void DrawList::Execute(SDL_Renderer* renderer) const {
    for (auto& cmd : drawCmds_) {
        switch (cmd.type) {
            case DrawCmd::Type::Clear:
                setDrawColor(renderer, cmd.clear.color);
                SDL_RenderClear(renderer);
                break;
            case DrawCmd::Type::Texture:
                executeTextureCmd(renderer, cmd.texture, cmd.beginIdx);
                break;
            case DrawCmd::Type::Geom:
                executeGeomCmd(renderer, cmd.geom, cmd.beginIdx);
                break;
        }
    }
}

void DrawList::setDrawColor(SDL_Renderer* renderer, const Color& c) const {
    SDL_SetRenderDrawColor(renderer, c.r * 255, c.g * 255, c.b * 255,
                           c.a * 255);
}

void DrawList::executeGeomCmd(SDL_Renderer* renderer, const GeomDrawCmd& cmd,
                              size_t startDataIdx) const {
    setDrawColor(renderer, cmd.color);
    switch (cmd.type) {
        case GeomDrawCmd::Type::Unknown:
            LOGW("unknown geometry draw command");
            break;
        case GeomDrawCmd::Type::Line:
            SDL_RenderDrawLineF(renderer, drawDatas_[startDataIdx],
                                drawDatas_[startDataIdx + 1],
                                drawDatas_[startDataIdx + 2],
                                drawDatas_[startDataIdx + 3]);
            break;
        case GeomDrawCmd::Type::LineStrip:
            SDL_RenderDrawLinesF(renderer,
                                 (SDL_FPoint*)&drawDatas_[startDataIdx],
                                 cmd.elemCount);
            break;
        case GeomDrawCmd::Type::LineLoop:
            SDL_RenderDrawLinesF(renderer,
                                 (SDL_FPoint*)&drawDatas_[startDataIdx],
                                 cmd.elemCount);
            SDL_RenderDrawLine(renderer, drawDatas_[startDataIdx],
                               drawDatas_[startDataIdx + 1],
                               drawDatas_[startDataIdx + cmd.elemCount - 1],
                               drawDatas_[startDataIdx + cmd.elemCount]);

            break;
        case GeomDrawCmd::Type::Rect:
            if (cmd.fill) {
                SDL_RenderFillRectsF(renderer,
                                     (SDL_FRect*)&drawDatas_[startDataIdx],
                                     cmd.elemCount);
            } else {
                SDL_RenderDrawRectsF(renderer,
                                     (SDL_FRect*)&drawDatas_[startDataIdx],
                                     cmd.elemCount);
            }
            break;
        case GeomDrawCmd::Type::Circle:
            for (size_t i = 0; i < cmd.elemCount; i++) {
                size_t idx = i * 3 + startDataIdx;
                Circle c{
                    Vec2{drawDatas_[idx], drawDatas_[idx + 1]},
                    drawDatas_[idx + 2]
                };
                constexpr int step = 10;
                constexpr float angle = 2 * PI / step;

                for (int j = 0; j < step; j++) {
                    float curAngle = j * angle;
                    float nextAngle = (j + 1) * angle;
                    Vec2 offset1{std::cos(curAngle), std::sin(curAngle)};
                    offset1 *= c.radius;
                    Vec2 offset2{std::cos(nextAngle), std::sin(nextAngle)};
                    offset2 *= c.radius;

                    Vec2 p1 = c.center + offset1;
                    Vec2 p2 = c.center + offset2;
                    SDL_RenderDrawLine(renderer, p1.x, p1.y, p2.x, p2.y);
                }
            }
            break;
    }
}

void DrawList::executeTextureCmd(SDL_Renderer* renderer,
                                 const TextureDrawCmd& cmd,
                                 size_t startDataIdx) const {
    TL_RETURN_IF_FALSE(cmd.texture);

    const Transform* trans = (const Transform*)&drawDatas_[startDataIdx];
    const Vec2* anchor = (const Vec2*)&drawDatas_[startDataIdx + 5];

    Rect dstRect;
    Vec2 unsignedScale = trans->scale;
    unsignedScale.x = std::abs(trans->scale.x);
    unsignedScale.y = std::abs(trans->scale.y);
    dstRect.size = unsignedScale * cmd.region.size;

    Vec2 xAxis = Rotate(Vec2::X_AXIS, trans->rotation);
    Vec2 yAxis = Rotate(Vec2::Y_AXIS, trans->rotation);
    int xSign = Sign(trans->scale.x);
    int ySign = Sign(trans->scale.y);
    Vec2 xOffset, yOffset;
    if (xSign > 0) {
        xOffset = -dstRect.size.w * anchor->x * xAxis;
    } else if (xSign < 0) {
        xOffset = -dstRect.size.w * (1.0 - anchor->x) * xAxis;
    }
    if (ySign > 0) {
        yOffset = -dstRect.size.h * anchor->y * yAxis;
    } else if (ySign < 0) {
        yOffset = -dstRect.size.h * (1.0 - anchor->y) * yAxis;
    }
    dstRect.position = trans->position + xOffset + yOffset;

    Flags f = Flip::None;

    Vec2 flipStatus{(cmd.flip & Flip::Horizontal) != Flip::None ? -1.0f : 1.0f,
                    (cmd.flip & Flip::Vertical) != Flip::None ? -1.0f : 1.0f};

    if (trans->scale.x * flipStatus.x < 0) {
        f |= Flip::Horizontal;
    }
    if (trans->scale.y * flipStatus.y < 0) {
        f |= Flip::Vertical;
    }

    SDL_Rect src;
    src.x = cmd.region.position.x;
    src.y = cmd.region.position.y;
    src.w = cmd.region.size.w;
    src.h = cmd.region.size.h;
    SDL_FPoint rotCenter{0, 0};
    SDL_SetTextureColorMod(cmd.texture->texture_, cmd.color.r * 255,
                           cmd.color.g * 255, cmd.color.b * 255);
    SDL_SetTextureAlphaMod(cmd.texture->texture_, cmd.color.a * 255);
    SDL_RenderCopyExF(
        renderer, cmd.texture->texture_, &src, (SDL_FRect*)&dstRect,
        trans->rotation, &rotCenter,
        static_cast<SDL_RendererFlip>(static_cast<uint32_t>(f)));
}

}  // namespace tl
