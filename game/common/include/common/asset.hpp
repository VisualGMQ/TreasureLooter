#pragma once
#include "rapidxml.hpp"
#include "common/uuid.hpp"
#include "common/path.hpp"

#include <memory>

template <typename T>
struct AssetLoadResult {
    UUID m_uuid;
    std::unique_ptr<T> m_payload;

    operator bool() const { return m_uuid && m_payload; }
};

template <typename T>
class AssetSLInfo {};

template <typename T>
AssetLoadResult<T> LoadAsset(const Path& filename);

template <typename T>
AssetLoadResult<T> LoadAsset(const rapidxml::xml_node<>&);

class IAssetManager {
public:
    virtual ~IAssetManager() = default;
    virtual const Path* GetFilename(const UUID& uuid) const  = 0;
    virtual bool IsExists(const Path& filename) const = 0;
    virtual bool IsExists(const UUID& uuid) const = 0;
};

