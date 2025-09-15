#pragma once
#include "schema/asset_info.hpp"

namespace internal {
template <typename... Args>
std::variant<std::monostate, Handle<Args>...> VariantAssetTypeGenerator(
    TypeList<Args...>);
}

using VariantAsset = decltype(internal::VariantAssetTypeGenerator(
    AssetTypeList{}));

void SaveVariantAsset(const VariantAsset& asset);
void SaveAsVariantAsset(const VariantAsset& asset, const Path&);
VariantAsset LoadVariantAsset(const Path& filename);

Path AppendExtension(const Path& path, const std::string& extension);

void SaveEntity(Entity entity);

class EntityPrefabComponent: public ComponentManager<PrefabHandle> {};