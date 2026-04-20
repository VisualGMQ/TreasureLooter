#include "server/asset_manager.hpp"
#include "server/font.hpp"
#include "server/image.hpp"

ImageManagerBase& ServerAssetsManager::getImageManager(TypeIndex type_index) {
    return ensureManager<ServerImageManager>(type_index);
}

FontManagerBase& ServerAssetsManager::getFontManager(TypeIndex type_index) {
    return ensureManager<TrivialFontManager>(type_index);
}
