#pragma once
#include "SDL3_ttf/SDL_ttf.h"
#include "common/asset_manager_interface.hpp"
#include "common/handle.hpp"
#include "common/math.hpp"
#include "common/path.hpp"

class FontBase {
public:
    virtual ~FontBase() = default;

    virtual bool IsValid() const noexcept = 0;
    virtual SDL_Surface* GenerateText(const std::string& text,
                                      const Color& color) const = 0;
    virtual int GetHeight() const = 0;
    virtual void SetFontSize(int) = 0;
};

template <>
class AssetSLInfo<FontBase> {
public:
    static constexpr bool CanEmbed = false;
};

using FontHandle = Handle<FontBase>;

class FontManagerBase: public AssetManagerBase<FontBase> {
public:
    virtual FontHandle Load(const Path& filename, bool force = false) = 0;
private:
};
