#pragma once
#include "SDL3_ttf/SDL_ttf.h"
#include "asset_manager_interface.hpp"
#include "handle.hpp"
#include "math.hpp"
#include "path.hpp"

class Font {
public:
    explicit Font(const Path& filename, int pt);

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
    Font(Font&&);
    Font& operator=(Font&&);

    ~Font();

    [[nodiscard]] SDL_Surface* GenerateText(const std::string& text,
                                            const Color& color) const;

    int GetHeight() const;

    void SetFontSize(int pt);

private:
    TTF_Font* m_font{};
};

using FontHandle = Handle<Font>;

class FontManager : public AssetManagerBase<Font> {
public:
    FontHandle Load(const Path& filename, bool force = false) override;
};