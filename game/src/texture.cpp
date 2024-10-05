#include "texture.hpp"
#include "context.hpp"
#include "log.hpp"
#include "macro.hpp"

namespace tl {

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

Texture* TextureManager::Load(const std::string& filename, const std::string& name) {
    auto it = textures_.find(name);
    if (it != textures_.end()) {
        LOGW("%s texture already exists(from %s)", name.c_str(), filename.c_str());
        return &it->second;
    }

    Texture texture(filename);
    TL_RETURN_NULL_IF(texture);

    return &textures_.emplace(name, std::move(texture)).first->second;
}

Texture* TextureManager::Get(const std::string& name) {
    return const_cast<Texture*>(std::as_const(*this).Get(name));
}

const Texture* TextureManager::Get(const std::string& name) const {
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
