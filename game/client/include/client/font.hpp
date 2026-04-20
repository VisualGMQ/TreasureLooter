#pragma once

#include "common/font.hpp"

class Font: public FontBase {
public:
    explicit Font(const Path& filename, int pt);

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
    Font(Font&&) noexcept;
    Font& operator=(Font&&) noexcept;

    ~Font();

    bool IsValid() const noexcept override { return m_font; }

    [[nodiscard]] SDL_Surface* GenerateText(const std::string& text,
                                            const Color& color) const override;

    int GetHeight() const override;

    void SetFontSize(int pt) override;

private:
    TTF_Font* m_font{};
};

class ClientFontManager : public FontManagerBase {
public:
    FontHandle Load(const Path& filename, bool force = false) override;
};
