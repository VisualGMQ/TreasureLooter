#include "server/font.hpp"

bool TrivialFont::IsValid() const noexcept {
    return false;
}

SDL_Surface* TrivialFont::GenerateText(const std::string& text,
                                       const Color& color) const {
    return nullptr;
}

int TrivialFont::GetHeight() const {
    return 0;
}

void TrivialFont::SetFontSize(int) {}

FontHandle TrivialFontManager::Load(const Path& filename, bool force) {
    return {};
}
