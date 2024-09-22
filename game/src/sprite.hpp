#pragma once
#include "pch.hpp"
#include "texture.hpp"
#include "transform.hpp"

namespace tl {

class Sprite {
public:
    operator bool() const {
        return texture_ && region_.size.w > 0 && region_.size.h > 0;
    }

    Transform transform;
    bool isEnable = true;

    void SetTexture(Texture& texture);
    Texture* GetTexture() const { return texture_; }
    void SetRegion(const Rect& region);
    const Rect& GetRegion() const { return region_; }
    void SetAnchor(const Vec2& anchor);
    const Vec2& GetAnchor() const { return anchor_; }

private:
    Texture* texture_ = nullptr;
    Rect region_;
    Vec2 anchor_ = Vec2{0.5, 0.5};
};

}
