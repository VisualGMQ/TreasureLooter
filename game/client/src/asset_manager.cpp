#include "client/asset_manager.hpp"
#include "client/context.hpp"
#include "client/font.hpp"
#include "client/image.hpp"
#include "common/image.hpp"
#include "common/type_index.hpp"

ImageManagerBase& ClientAssetsManager::getImageManager(TypeIndex type_index) {
    return ensureManager<ClientImageManager>(type_index,
                                             *CLIENT_CONTEXT.m_renderer);
}

FontManagerBase& ClientAssetsManager::getFontManager(TypeIndex type_index) {
    return ensureManager<ClientFontManager>(type_index);
}
