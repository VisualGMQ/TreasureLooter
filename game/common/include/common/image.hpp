#pragma once
#include "SDL3/SDL.h"
#include "common/math.hpp"
#include "common/asset_manager_interface.hpp"

class ImageBase {
public:
    ImageBase() = default;
    virtual ~ImageBase() = default;

    virtual Vec2 GetSize() const = 0;
    [[nodiscard]] virtual SDL_Texture* GetTexture() const = 0;
    virtual void ChangeColorMask(const Color&) = 0;
};

template <>
class AssetSLInfo<ImageBase> {
public:
    static constexpr bool CanEmbed = false;
};

using ImageHandle = Handle<ImageBase>;

class ImageManagerBase: public AssetManagerBase<ImageBase> {
public:
    virtual ImageHandle Load(const Path& filename, bool force = false) = 0;
};

