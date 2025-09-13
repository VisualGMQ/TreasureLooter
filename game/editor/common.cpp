#include "common.hpp"

#include "context.hpp"
#include "engine/asset_manager.hpp"
#include "schema/serialize/serialize.hpp"

namespace internal {

struct AssetLoader {
    template <typename... Args>
    VariantAsset operator()(TypeList<Args...>, const Path& filename) {
        VariantAsset asset;
        std::string filename_str = filename.string();
        std::string extension =
            filename_str.substr(filename_str.find_first_of('.'));
        (checkAndLoad<Args>(filename, extension, asset), ...);
        return asset;
    }

private:
    template <typename T>
    void checkAndLoad(const Path& filename, const std::string& extension,
                      VariantAsset& out_asset) {
        if (AssetInfoManager::GetExtension<T>() == extension) {
            out_asset = EDITOR_CONTEXT.m_assets_manager->GetManager<T>().Load(
                filename, true);
        }
    }
};

}  // namespace internal

template <typename>
struct ShowTemplate;

void SaveVariantAsset(const VariantAsset& asset) {
    std::visit(
        [](auto payload) {
            using type = std::decay_t<decltype(payload)>;
            if constexpr (!std::is_same_v<type, std::monostate>) {
                SaveAsset(payload.GetUUID(), *payload.Get(),
                          *payload.GetFilename());
            }
        },
        asset);
}

void SaveAsVariantAsset(const VariantAsset& asset, const Path& filename) {
    std::visit(
        [&](auto payload) {
            using type = std::decay_t<decltype(payload)>;
            if constexpr (!std::is_same_v<type, std::monostate>) {
                SaveAsset(UUID::CreateV4(), *payload.Get(), filename);
            }
        },
        asset);
}

VariantAsset LoadVariantAsset(const Path& filename) {
    internal::AssetLoader loader{};
    return loader(AssetTypeList{}, filename);
}

Path AppendExtension(const Path& path, const std::string& extension) {
    std::string filename = path.string();
    if (path.has_extension()) {
        auto ext = filename.substr(filename.find_last_of("."));
        if (extension != ext) {
            filename += extension;
        }
    } else {
        filename += extension;
    }

    return filename;
}
