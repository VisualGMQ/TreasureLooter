#pragma once
#include "pch.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "transform.hpp"

namespace tl {

class Sprite {
public:
    operator bool() const {
        return isTextureValid() && region_.size.w > 0 && region_.size.h > 0;
    }

    Vec2 anchor = Vec2{0.5, 0.5};
    Flags<Flip> flip = Flip::None;
    bool enable = true;
    Color color = Color::White;

    void SetTexture(Texture& texture);
    const Texture* GetTexture() const;
    Texture* GetTexture();

    void SetFontTexture(FontTexture&& texture);
    void SetText(const std::string& text);
    void SetFontSize(uint8_t size);
    void SetFont(Font& font);
    std::string GetText() const;
    uint8_t GetFontSize();
    Font* GetFont();

    void SetRegion(const Rect& region);
    const Rect& GetRegion() const { return region_; }
    bool IsTexture() const;
    bool IsText() const;

private:
    std::variant<Texture*, FontTexture> texture_;

    Rect region_;

    bool isTextureValid() const;
    void updateRegionFromFontTexture();
    const FontTexture* getFontTexture() const;
    FontTexture* getFontTexture();
};

}  // namespace tl
