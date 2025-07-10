#include "image.hpp"
#include "log.hpp"
#include "renderer.hpp"
#include "sdl_call.hpp"
#include "stb_image.h"

Image::Image(Renderer& renderer, const Path& filename) {
    int w, h;
    stbi_uc* data =
        stbi_load(filename.string().c_str(), &w, &h, nullptr, STBI_rgb_alpha);
    if (!data) {
        LOGE("load image {} failed", filename);
        return;
    }

    SDL_Renderer* sdl_renderer = renderer.GetRenderer();
    SDL_Surface* surface =
        SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, data, w * 4);
    if (!surface) {
        LOGE("create SDL surface from {} failed: {}", filename, SDL_GetError());
        stbi_image_free(data);
        return;
    }
    m_texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    if (!m_texture) {
        LOGE("create SDL texture from {} failed: {}", filename, SDL_GetError());
        stbi_image_free(data);
        SDL_DestroySurface(surface);
        return;
    }
    SDL_DestroySurface(surface);

    SDL_CALL(SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND));
    stbi_image_free(data);
    
    SDL_SetTextureScaleMode(m_texture, SDL_SCALEMODE_NEAREST);
}

Image::~Image() {
    SDL_DestroyTexture(m_texture);
}

Vec2 Image::GetSize() const {
    Vec2 size;
    SDL_CALL(SDL_GetTextureSize(m_texture, &size.w, &size.h));
    return size;
}

SDL_Texture* Image::GetTexture() const {
    return m_texture;
}

ImageManager::ImageManager(Renderer& renderer) : m_renderer{renderer} {}

Image* ImageManager::Load(const Path& filename) {
    if (auto it = m_images.find(filename); it != m_images.end()) {
        LOGW("image {} already loaded", filename);
        return it->second.get();
    }

    auto result = m_images.emplace(
        filename, std::make_unique<Image>(m_renderer, filename));
    if (!result.second) {
        LOGE("emplace image failed");
        return nullptr;
    }

    return result.first->second.get();
}

Image* ImageManager::Find(const Path& filename) {
    if (auto it = m_images.find(filename); it != m_images.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool ImageManager::IsExists(const Path& filename) {
    return Find(filename);
}