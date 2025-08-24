#include "text.hpp"

#include "log.hpp"
#include "sdl_call.hpp"
#include "storage.hpp"
#include "SDL3_ttf/SDL_ttf.h"

Font::Font(const Path& filename, int pt) {
    m_font = TTF_OpenFont(filename.string().c_str(), pt);
    if (!m_font) {
        LOGE("open font file {} failed: {}", filename.string(), SDL_GetError());
    }
}

Font::~Font() {
    TTF_CloseFont(m_font);
}

SDL_Surface* Font::GenerateText(const std::string& text, const Color& color) const {
    SDL_Color sdl_color;
    sdl_color.r = color.r * 255;
    sdl_color.g = color.g * 255;
    sdl_color.b = color.b * 255;
    sdl_color.a = color.a * 255;
    SDL_Surface* surface = TTF_RenderText_Blended(m_font, text.c_str(),
                                                  text.size(), sdl_color);
    if (!surface) {
        LOGW("create text surface from text \"{}\" failed: {}", text,
             SDL_GetError());
    }
    return surface;
}

int Font::GetHeight() const {
    return TTF_GetFontHeight(m_font);
}

void Font::SetFontSize(int pt) {
    SDL_CALL(TTF_SetFontSize(m_font, pt));
}

FontHandle FontManager::Load(const Path& filename, bool force) {
    if (auto it = Find(filename); it && !force) {
        LOGW("font {} already loaded", filename);
        return it;
    }

    return store(&filename, UUID::CreateV4(),
                 std::make_unique<Font>(filename, 16));
}
