#include "font.hpp"
#include "macro.hpp"

namespace tl {

Font::Font(const std::string& filename, uint8_t ptSize) : ptSize_{ptSize} {
    font_ = TTF_OpenFont(filename.c_str(), ptSize);
    TL_RETURN_IF_FALSE_LOGE(font_, "font %s load failed: %s", filename.c_str(),
                      TTF_GetError());
}

Font::Font(Font&& o) : font_{o.font_}, ptSize_{o.ptSize_} {
    o.font_ = nullptr;
    o.ptSize_ = 0;
}

Font& Font::operator=(Font&& o) noexcept {
    if (&o != this) {
        font_ = o.font_;
        ptSize_ = o.ptSize_;
        o.font_ = nullptr;
        o.ptSize_ = 0;
    }
    return *this;
}

void Font::SetPtSize(uint8_t size) const {
    TL_RETURN_IF_FALSE(size > 0);
    TTF_SetFontSize(font_, size);
}

Font::~Font() {
    TTF_CloseFont(font_);
}

Font* FontManager::Load(const std::string& filename, const std::string& name) {
    Font font{filename, 16};
    TL_RETURN_NULL_IF_FALSE_LOGE(font, "load %s failed", filename.c_str());
    auto result = fonts_.emplace(name, std::move(font));
    TL_RETURN_NULL_IF_FALSE_LOGE(result.second, "emplace font %s failed",
                           filename.c_str());
    return &result.first->second;
}

const Font* FontManager::Find(const std::string& name) const {
    auto it = fonts_.find(name);
    TL_RETURN_NULL_IF_FALSE(fonts_.end() != it);
    return &it->second;
}

Font* FontManager::Find(const std::string& name) {
    return const_cast<Font*>(std::as_const(*this).Find(name));
}

void FontManager::Destroy(const std::string& name) {
    fonts_.erase(name);
}

void FontManager::Clear() {
    fonts_.clear();
}

}  // namespace tl