#pragma once
#include "uuid.hpp"
#include "path.hpp"
#include "rapidxml.hpp"

template <typename T>
struct AssetLoadResult {
    UUIDv4 m_uuid;
    T m_payload;

    operator bool() const {
        return m_uuid;
    }
};

template <typename T>
AssetLoadResult<T> LoadAsset(const Path& filename);

template <typename T>
AssetLoadResult<T> LoadAsset(rapidxml::xml_node<>&);

class IAssetManager {
public:
    virtual ~IAssetManager() = default;
    virtual const Path* GetFilename(const UUIDv4& uuid) const  = 0;
    virtual bool IsExists(const Path& filename) const = 0;
    virtual bool IsExists(const UUIDv4& uuid) const = 0;
};

