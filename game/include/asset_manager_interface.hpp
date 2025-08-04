#pragma once
#include "handle.hpp"

#include "asset.hpp"
#include "log.hpp"
#include "path.hpp"
#include "type_index.hpp"
#include "uuid.hpp"

#include <memory>
#include <unordered_map>

class Tilemap;
class ImageManager;
class Image;

template <typename T>
class AssetManagerBase : public IAssetManager {
public:
    using HandleType = Handle<T>;

    virtual ~AssetManagerBase() = default;

    virtual HandleType Load(const Path& filename) = 0;

    HandleType Find(const Path& filename) {
        if (auto it = m_paths_uuid_map.find(filename);
            it != m_paths_uuid_map.end()) {
            return Find(it->second);
        }
        return nullptr;
    }

    HandleType Find(UUID uuid) {
        if (auto it = m_payloads.find(uuid); it != m_payloads.end()) {
            return {it->first, it->second.get(), this};
        }
        return nullptr;
    }

    bool IsExists(const Path& filename) const override {
        return m_paths_uuid_map.find(filename) != m_paths_uuid_map.end();
    }

    bool IsExists(const UUID& uuid) const override {
        return m_payloads.find(uuid) != m_payloads.end();
    }

    const Path* GetFilename(const UUID& uuid) const override {
        if (auto it = m_uuid_path_map.find(uuid); it != m_uuid_path_map.end()) {
            return &it->second;
        }
        return nullptr;
    }

protected:
    HandleType store(const Path* filename, UUID uuid,
                     std::unique_ptr<T>&& payload) {
        auto result = m_payloads.emplace(uuid, std::move(payload));
        if (!result.second) {
            LOGE("load asset {} failed",
                 filename ? *filename : "<no filename>");
            return nullptr;
        }

        if (filename) {
            auto r = m_paths_uuid_map.emplace(*filename, uuid);
            if (!r.second) {
                LOGE("emplace filename failed: {}", *filename);
                return nullptr;
            }

            m_uuid_path_map.emplace(uuid, *filename);
        }
        return HandleType{uuid, result.first->second.get(), this};
    }

private:
    std::unordered_map<UUID, std::unique_ptr<T>> m_payloads;
    std::unordered_map<Path, UUID> m_paths_uuid_map;
    std::unordered_map<UUID, Path> m_uuid_path_map;
};

template <typename T>
class GenericAssetManager : public AssetManagerBase<T> {
public:
    using HandleType = typename AssetManagerBase<T>::HandleType;

    HandleType Load(const Path& filename) override {
        auto result = LoadAsset<T>(filename);
        return store(&filename, result.m_uuid,
                     std::make_unique<T>(std::move(result.m_payload)));
    }
};