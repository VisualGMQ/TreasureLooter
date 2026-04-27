#pragma once

#include "common/image.hpp"

// trivial, don't do anything, just placeholder
class ServerImage : public ImageBase {
public:
    Vec2 GetSize() const override;
    SDL_Texture* GetTexture() const override;
    void ChangeColorMask(const Color&) override;
};

// trivial, don't do anything, just placeholder
class ServerImageManager : public ImageManagerBase {
public:
    ImageHandle Load(const Path& filename, bool force = false) override;
};
