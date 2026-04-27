#pragma once

#include "common/asset_manager.hpp"

class ServerAssetsManager: public IAssetsManager {
protected:
    ImageManagerBase& getImageManager(TypeIndex) override;
    FontManagerBase& getFontManager(TypeIndex) override;
};
