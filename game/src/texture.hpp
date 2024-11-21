#pragma once
#include "math.hpp"
#include "pch.hpp"
#include "font.hpp"

namespace tl {

class Texture {
public:
    friend class Renderer;
    friend class Inspector;

    Texture() = default;
    Texture(SDL_Texture*);
    Texture(SDL_Surface*);
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

struct FontTexture {
public:
    FontTexture() = default;
    FontTexture(Font& font, const std::string& text, uint8_t fontSize);

    const Texture* GetTexture() const { return texture_.get(); }

    const std::string& GetText() const { return text_; }

    void ChangeText(const std::string& text);
    void ChangeFont(Font& font);
    void ChangeFontSize(uint8_t size);
    void ChangeTextAndFont(const std::string& text, Font& font, uint8_t size);
    uint8_t GetFontSize() const { return fontSize_; }
    Font* GetFont() { return font_; }
    const Font* GetFont() const { return font_; }

    operator bool() const { return font_ && texture_; }

private:
    std::unique_ptr<Texture> texture_;
    Font* font_ = nullptr;
    uint8_t fontSize_ = 16;
    std::string text_;

    void changeTextAndFont(const std::string* newText, Font* font, uint8_t size);
};

class TextureManager {
public:
    Texture* Load(const std::string& filename, const std::string& name);
    const Texture* Find(const std::string& name) const;
    Texture* Find(const std::string& name);
    void Destroy(const std::string& name);
    void Clear();

private:
    std::unordered_map<std::string, Texture> textures_;
};

}  // namespace tl
