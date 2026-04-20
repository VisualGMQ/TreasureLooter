#pragma once

#include "common/font.hpp"

class TrivialFont : public FontBase {
public:
    bool IsValid() const noexcept override;

    SDL_Surface* GenerateText(const std::string& text,
                              const Color& color) const override;

    int GetHeight() const override;

    void SetFontSize(int) override;
};

class TrivialFontManager: public FontManagerBase {
public:
    FontHandle Load(const Path& filename, bool force = false) override;
};
