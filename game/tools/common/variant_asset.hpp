#pragma once
#include "engine/dialog.hpp"
#include "schema/asset_info.hpp"

class ToolContext;

namespace internal {
template <typename... Args>
std::variant<std::monostate, Handle<Args>...> VariantAssetTypeGenerator(
    TypeList<Args...>);
}

using VariantAsset = decltype(internal::VariantAssetTypeGenerator(
    AssetTypeList{}));

void SaveVariantAsset(const VariantAsset& asset);
void SaveAsVariantAsset(const VariantAsset& asset, const Path&);
VariantAsset LoadVariantAsset(const Path& filename, ToolContext&);

Filter GetAssetFilterByType(const VariantAsset& asset);

const Path* GetAssetFilename(const VariantAsset& asset);

std::vector<Filter> GetAssetFilters();
