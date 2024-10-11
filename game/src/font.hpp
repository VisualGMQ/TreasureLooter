#pragma once
#include "id.hpp"
#include "math.hpp"
#include "pch.hpp"

namespace tl {

class Font {
public:
    friend class FontTexture;

    Font(const std::string& filename, uint8_t ptSize);
    Font(const Font&) = delete;
    Font(Font&&);
    Font& operator=(const Font&) = delete;
    Font& operator=(Font&&) noexcept;
    ~Font();

    uint8_t GetPtSize() const { return ptSize_; }
    void SetPtSize(uint8_t size) const;

    operator bool() const { return font_; }

private:
    TTF_Font* font_ = nullptr;
    uint8_t ptSize_;
};

class FontManager {
public:
    Font* Load(const std::string& filename, const std::string& name);
    const Font* Find(const std::string& name) const;
    Font* Find(const std::string& name);
    void Destroy(const std::string& name);
    void Clear();

private:
    std::unordered_map<std::string, Font> fonts_;
};

}  // namespace tl