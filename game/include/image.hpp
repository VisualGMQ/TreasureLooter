#pragma once
#include "SDL3/SDL.h"
#include "asset_manager_interface.hpp"
#include "math.hpp"
#include "path.hpp"

class Renderer;

class Image {
public:
    Image(Renderer& Renderer, const Path& filename);
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&&) noexcept;
    Image& operator=(Image&&) noexcept;
    ~Image();

    Vec2 GetSize() const;

    SDL_Texture* GetTexture() const;

private:
    SDL_Texture* m_texture{};
};

using ImageHandle = Handle<Image>;

class ImageManager: public AssetManagerBase<Image> {
public:
    explicit ImageManager(Renderer& renderer);

    ImageHandle Load(const Path& filename, bool force = false) override;

private:
    Renderer& m_renderer;
};