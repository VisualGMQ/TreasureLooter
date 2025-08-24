#include "text.hpp"

#include "log.hpp"
#include "storage.hpp"
#include "SDL3_ttf/SDL_ttf.h"

Font::Font(const Path& filename) {
    TTF_Init();
}

std::vector<char> Font::GenerateText(const std::string& text,
                                     uint32_t pixel) const {
    return {};
}
