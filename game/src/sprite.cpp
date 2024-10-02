#include "sprite.hpp"
#include "log.hpp"

namespace tl {

void Sprite::SetTexture(Texture& texture) {
    texture_ = &texture;
    region_.position = Vec2::ZERO;
    region_.size = texture_->GetSize();
    anchor = Vec2{0.5, 0.5};
}

void Sprite::SetRegion(const Rect& region) {
    if (!texture_) {
        return;
    }

    Vec2 size = texture_->GetSize();
    if (region.position.x < 0 || region.position.y < 0 || region.size.w < 0 ||
        region.size.h < 0 || region.position.x + region.size.w > size.w ||
        region.position.y + region.size.h > size.h) {
        LOGW("invalid region in sprite");
        return;
    }

    region_ = region;
}

}  // namespace tl
