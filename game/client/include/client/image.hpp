#pragma once
#include "common/image.hpp"

class Renderer;

class Image: public ImageBase {
public:
    Image() = default;
    Image(Renderer& renderer, SDL_Surface* surface);
    Image(Renderer& renderer, const Path& filename);
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&&) noexcept;
    Image& operator=(Image&&) noexcept;
    ~Image();

    Vec2 GetSize() const;

    [[nodiscard]] SDL_Texture* GetTexture() const;
    void ChangeColorMask(const Color& color);

private:
    SDL_Texture* m_texture{};
};

class ClientImageManager: public ImageManagerBase {
public:
    explicit ClientImageManager(Renderer& renderer);

    ImageHandle Load(const Path& filename, bool force = false) override;

private:
    Renderer& m_renderer;
};
