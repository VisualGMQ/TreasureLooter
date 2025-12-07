#include "context.hpp"
#include "engine/asset_manager.hpp"
#include "engine/handle.hpp"
#include "rapidxml.hpp"
#include "schema/asset_info.hpp"
#include "schema/serialize/serialize.hpp"
#include "variant_asset.hpp"

VariantAsset LoadVariantAsset(const Path& filename) {
    return AssetInfoManager::LoadAsset(filename, ASSET_VIEWER_CONTEXT);
}

struct AssetSaver {
public:
    AssetSaver() = default;

    AssetSaver(const Path& save_as_path)
        : m_is_save_as{true}, m_save_as_path{save_as_path} {}

    bool IsSaveAs() const { return m_is_save_as; }

    void operator()(std::monostate) {}

    template <typename T, typename = std::enable_if_t<is_handle_v<T>>>
    void operator()(const T& handle) {
        using payload_type = typename T::underlying_type;

        if (m_is_save_as) {
            SaveAsset(handle.GetUUID(), *handle, m_save_as_path);
        } else {
            SaveAsset(handle.GetUUID(), *handle, *handle.GetFilename());
        }
    }

private:
    bool m_is_save_as = false;
    Path m_save_as_path;
};

void SaveVariantAsset(const VariantAsset& asset) {
    std::visit(AssetSaver{}, asset);
}

void SaveAsVariantAsset(const VariantAsset& asset, const Path& save_as_path) {
    AssetSaver saver{save_as_path};
    std::visit(saver, asset);
}

Filter GetAssetFilterByType(const VariantAsset& asset) {
    return std::visit(
        [](const auto& payload) {
            using type = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<type, std::monostate>) {
                return Filter{};
            } else {
                using payload_type = typename type::underlying_type;
                auto extension = AssetInfoManager::GetExtension<payload_type>();
                auto name = AssetInfoManager::GetName<payload_type>();
                return Filter{name.data(), extension.data()};
            }
        },
        asset);
}

struct GetAssetFilenameHelper {
    void operator()(std::monostate) {}

    template <typename T>
    void operator()(T handle) {
        m_filename = handle.GetFilename();
    }

    const Path* m_filename = nullptr;
};

const Path* GetAssetFilename(const VariantAsset& asset) {
    GetAssetFilenameHelper helper;
    std::visit(helper, asset);
    return helper.m_filename;
}

std::vector<Filter> GetAssetFilters() {
    std::vector<Filter> filters;
    for (size_t i = 0; i < AssetInfoManager::GetNum(); i++) {
        std::string_view extension = AssetInfoManager::GetExtensions()[i];
        std::string_view type_name = AssetInfoManager::GetNames()[i];

        Filter filter;
        filter.name = type_name;
        filter.pattern = extension;
        filters.push_back(filter);
    }
    return filters;
}
