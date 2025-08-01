#pragma once
#include "rapidxml.hpp"
#include "uuid.hpp"
#include "path.hpp"

template <typename T>
struct AssetLoadResult {
    UUID m_uuid;
    T m_payload;
};

template <typename T>
AssetLoadResult<T> LoadAsset(const Path& filename);

template <typename T>
AssetLoadResult<T> LoadAsset(rapidxml::xml_node<>&);

class IAssetManager {
public:
    virtual ~IAssetManager() = default;
    virtual const Path* GetFilename(const UUID& uuid) const  = 0;
    virtual bool IsExists(const Path& filename) const = 0;
    virtual bool IsExists(const UUID& uuid) const = 0;
};

