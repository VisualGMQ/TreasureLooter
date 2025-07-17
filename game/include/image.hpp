#pragma once
#include "SDL3/SDL.h"
#include "math.hpp"
#include <unordered_map>
#include "path.hpp"

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

class ImageManager {
public:
    explicit ImageManager(Renderer& renderer);
    
    Image* Load(const Path& filename);
    Image* Find(const Path& filename);
    bool IsExists(const Path& filename);
    
private:
    std::unordered_map<Path, std::unique_ptr<Image>> m_images;

    Renderer& m_renderer;
};