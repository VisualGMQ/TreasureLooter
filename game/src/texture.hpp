#pragma once
#include "math.hpp"
#include "pch.hpp"

namespace tl {

class Texture {
public:
    friend class Renderer;

    Texture(const std::string& filename);
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&);
    Texture& operator=(Texture&&);
    ~Texture();

    const Vec2& GetSize() const { return size_; }

    operator bool() const;

private:
    SDL_Texture* texture_;
    Vec2 size_;
};

class TextureManager {
public:
    Texture* Load(const std::string& filename);
    Texture* Get(const std::string& filename) const;
    void Destroy(const std::string& filename);
    void Clear();

private:
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
};

}  // namespace tl
