#include "server/image.hpp"

Vec2 ServerImage::GetSize() const {
    return {};
}

SDL_Texture* ServerImage::GetTexture() const {
    return nullptr;
}

void ServerImage::ChangeColorMask(const Color&) {}

ImageHandle ServerImageManager::Load(const Path& filename, bool force) {
    return {};
}


