#include "image.hpp"
#include "log.hpp"
#include "renderer.hpp"
#include "sdl_call.hpp"
#include "stb_image.h"
#include "storage.hpp"

Image::Image(Renderer& renderer, const Path& filename) {
    int w, h;

    auto file = IOStream::CreateFromFile(filename, IOMode::Read, true);
    auto content = file->Read();

    stbi_uc* data =
        stbi_load_from_memory((const stbi_uc*)content.data(), content.size(),
                              &w, &h, nullptr, STBI_rgb_alpha);
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

Image::Image(Image&& o) noexcept : m_texture{o.m_texture} {
    o.m_texture = nullptr;
}

Image& Image::operator=(Image&& o) noexcept {
    if (&o != this) {
        m_texture = o.m_texture;
        o.m_texture = nullptr;
    }
    return *this;
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

ImageHandle ImageManager::Load(const Path& filename, bool force) {
    if (auto it = Find(filename); it && !force) {
        LOGW("image {} already loaded", filename);
        return it;
    }

    return store(&filename, UUID::CreateV4(),
                 std::make_unique<Image>(m_renderer, filename));
}