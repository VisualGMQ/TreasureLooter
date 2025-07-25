#pragma once
#include "SDL3/SDL.h"
#include "asset_manager.hpp"
#include "math.hpp"
#include "path.hpp"
#include <unordered_map>

class Renderer;

class Image {
public:
    Image(Renderer& Renderer, const Path& filename);
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&&) = delete;
    Image& operator=(Image&&) = delete;
    ~Image();

    Vec2 GetSize() const;

    SDL_Texture* GetTexture() const;

    const Path& Filename() const;

private:
    SDL_Texture* m_texture{};
    Path m_filename;
};

using ImageHandle = Handle<Image>;

class ImageManager: public AssetManagerBase<Image> {
public:
    explicit ImageManager(Renderer& renderer);
    
    ImageHandle Load(const Path& filename) override;

private:
    Renderer& m_renderer;
};