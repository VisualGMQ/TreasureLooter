#include "sprite.hpp"
#include "log.hpp"
#include "macro.hpp"

namespace tl {

void Sprite::SetTexture(Texture& texture) {
    texture_ = &texture;
    region_.position = Vec2::ZERO;
    region_.size = texture.GetSize();
    anchor = Vec2{0.5, 0.5};
}

void Sprite::SetFontTexture(FontTexture&& texture) {
    const Texture* innerTexture = texture.GetTexture();
    TL_RETURN_IF_FALSE(innerTexture);

    region_.size = innerTexture->GetSize();
    region_.position = Vec2::ZERO;

    texture_ = std::move(texture);
}

void Sprite::SetText(const std::string& text) {
    auto fontTexture = getFontTexture();
    TL_RETURN_IF_FALSE(fontTexture);

    fontTexture->ChangeText(text);
    updateRegionFromFontTexture();
}

void Sprite::SetFontSize(uint8_t size) {
    auto fontTexture = getFontTexture();
    TL_RETURN_IF_FALSE(fontTexture);

    fontTexture->ChangeFontSize(size);
    updateRegionFromFontTexture();
}

void Sprite::SetFont(Font& font) {
    auto fontTexture = getFontTexture();
    TL_RETURN_IF_FALSE(fontTexture);

    fontTexture->ChangeFont(font);
    updateRegionFromFontTexture();
}

std::string Sprite::GetText() const {
    auto fontTexture = getFontTexture();
    TL_RETURN_VALUE_IF_FALSE(fontTexture, {});

    return fontTexture->GetText();
}

uint8_t Sprite::GetFontSize() {
    auto fontTexture = getFontTexture();
    TL_RETURN_VALUE_IF_FALSE(fontTexture, {});

    return fontTexture->GetFontSize();
}

Font* Sprite::GetFont() {
    auto fontTexture = getFontTexture();
    TL_RETURN_VALUE_IF_FALSE(fontTexture, {});

    return fontTexture->GetFont();
}

void Sprite::updateRegionFromFontTexture() {
    auto fontTexture = getFontTexture();
    TL_RETURN_IF_FALSE(fontTexture && fontTexture->GetTexture());
    region_.position = Vec2::ZERO;
    region_.size = fontTexture->GetTexture()->GetSize();
}

const Texture* Sprite::GetTexture() const {
    auto texture = std::get_if<Texture*>(&texture_);
    if (texture) {
        return *texture;
    }

    auto fontTexture = std::get_if<FontTexture>(&texture_);
    if (fontTexture) {
        return fontTexture->GetTexture();
    }

    return nullptr;
}

Texture* Sprite::GetTexture() {
    return const_cast<Texture*>(std::as_const(*this).GetTexture());
}

const FontTexture* Sprite::getFontTexture() const {
    auto texture = std::get_if<FontTexture>(&texture_);
    return texture;
}

FontTexture* Sprite::getFontTexture() {
    return const_cast<FontTexture*>(std::as_const(*this).getFontTexture());
}

void Sprite::SetRegion(const Rect& region) {
    auto texture = GetTexture();

    TL_RETURN_IF_FALSE(texture);

    Vec2 size = texture->GetSize();
    if (region.position.x < 0 || region.position.y < 0 || region.size.w < 0 ||
        region.size.h < 0 || region.position.x + region.size.w > size.w ||
        region.position.y + region.size.h > size.h) {
        LOGW("invalid region in sprite");
        return;
    }

    region_ = region;
}

bool Sprite::IsTexture() const {
    return std::get_if<Texture*>(&texture_);
}

bool Sprite::IsText() const {
    return std::get_if<FontTexture>(&texture_);
}

bool Sprite::isTextureValid() const {
    auto texture = std::get_if<Texture*>(&texture_);
    if (texture) {
        return *texture;
    }

    auto fontTexture = std::get_if<FontTexture>(&texture_);
    if (fontTexture) {
        return *fontTexture;
    }

    return false;
}

}  // namespace tl
