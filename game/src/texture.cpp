#include "texture.hpp"
#include "context.hpp"
#include "log.hpp"
#include "macro.hpp"

namespace tl {

Texture::Texture(SDL_Texture* texture) : texture_{texture} {
    TL_RETURN_IF(texture);

    int w, h;
    SDL_QueryTexture(texture_, nullptr, nullptr, &w, &h);
    size_.w = w;
    size_.h = h;
}

Texture::Texture(SDL_Surface* surface) {
    TL_RETURN_IF(surface);
    texture_ = SDL_CreateTextureFromSurface(
        Context::GetInst().renderer->renderer_, surface);
    TL_RETURN_IF(texture_);

    size_.w = surface->w;
    size_.h = surface->h;
}

Texture::Texture(const std::string& filename) {
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        LOGW("load texture %s failed: %s", filename.c_str(), IMG_GetError());
    } else {
        SDL_Surface* newSurface =
            SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
        if (!newSurface) {
            LOGW("convert surface %s to RGBA8888 failed: %s", filename.c_str(),
                 SDL_GetError());
        }
        SDL_FreeSurface(surface);
        texture_ = SDL_CreateTextureFromSurface(
            Context::GetInst().renderer->renderer_, newSurface);
        if (!texture_) {
            LOGW("create texture %s from surface failed: %s", filename.c_str(),
                 SDL_GetError());
        } else {
            size_.w = newSurface->w;
            size_.h = newSurface->h;
        }
        SDL_FreeSurface(newSurface);
    }
}

Texture::Texture(Texture&& o) : texture_{o.texture_}, size_{o.size_} {
    o.texture_ = nullptr;
}

Texture::~Texture() {
    SDL_DestroyTexture(texture_);
}

Texture& Texture::operator=(Texture&& o) {
    if (&o != this) {
        texture_ = o.texture_;
        size_ = o.size_;
        o.texture_ = nullptr;
    }
    return *this;
}

Texture::operator bool() const {
    return texture_;
}

FontTexture::FontTexture(Font& font, const std::string& text, uint8_t fontSize)
    : font_{&font}, text_{text}, fontSize_{fontSize} {
    changeTextAndFont(&text, &font, fontSize_);
}

void FontTexture::ChangeFont(Font& font) {
    TL_RETURN_IF(&font != font_);

    changeTextAndFont(nullptr, &font, fontSize_);
}

void FontTexture::ChangeTextAndFont(const std::string& text, Font& font,
                                    uint8_t size) {
    TL_RETURN_IF(text != text_ && &font != font_ && size != fontSize_);

    changeTextAndFont(&text, &font, size);
}

void FontTexture::ChangeText(const std::string& text) {
    TL_RETURN_IF(text_ != text);

    changeTextAndFont(&text, nullptr, fontSize_);
}

void FontTexture::ChangeFontSize(uint8_t size) {
    TL_RETURN_IF(size != fontSize_);

    changeTextAndFont(&text_, font_, size);
}



void FontTexture::changeTextAndFont(const std::string* text, Font* font,
                                    uint8_t size) {
    if (font) {
        font_ = font;
    }

    if (text) {
        text_ = *text;
    }

    fontSize_ = size;

    TL_RETURN_IF(!text->empty());

    font_->SetPtSize(size);

    SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(
        font_->font_, text_.c_str(), SDL_Color{255, 255, 255, 255}, 0);
    TL_RETURN_IF(surface);

    texture_ = std::make_unique<Texture>(surface);
    SDL_FreeSurface(surface);

    TL_RETURN_IF_LOGE(texture_, "font texture create failed with text %s",
                      text_.c_str());
}

Texture* TextureManager::Load(const std::string& filename,
                              const std::string& name) {
    auto it = textures_.find(name);
    if (it != textures_.end()) {
        LOGW("%s texture already exists(from %s)", name.c_str(),
             filename.c_str());
        return &it->second;
    }

    Texture texture(filename);
    TL_RETURN_NULL_IF(texture);

    return &textures_.emplace(name, std::move(texture)).first->second;
}

Texture* TextureManager::Find(const std::string& name) {
    return const_cast<Texture*>(std::as_const(*this).Find(name));
}

const Texture* TextureManager::Find(const std::string& name) const {
    if (auto it = textures_.find(name); it != textures_.end()) {
        return &it->second;
    }
    return nullptr;
}

void TextureManager::Destroy(const std::string& name) {
    textures_.erase(name);
}

void TextureManager::Clear() {
    textures_.clear();
}

}  // namespace tl
