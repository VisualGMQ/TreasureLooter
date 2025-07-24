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
