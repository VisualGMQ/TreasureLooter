#pragma once
#include "common/handle.hpp"

#include "common/asset.hpp"
#include "common/log.hpp"
#include "common/path.hpp"
#include "common/type_index.hpp"
#include "common/uuid.hpp"

#include <memory>
#include <unordered_map>

#include "macros.hpp"

class Tilemap;
class ImageManager;
class Image;

template <typename T>
class AssetManagerBase : public IAssetManager {
public:
    using HandleType = Handle<T>;

    virtual ~AssetManagerBase() = default;

    // load asset from path
    virtual HandleType Load(const Path& filename, bool force = false) = 0;

    // create an empty asset
    HandleType Create() {
        return this->store(nullptr, UUID::CreateV4(), std::make_unique<T>());
    }

    HandleType Create(const UUID& uuid, T&& value, const Path* filename) {
        TL_RETURN_DEFAULT_IF_FALSE(uuid);
        return this->store(filename, uuid,
                           std::make_unique<T>(std::forward<T>(value)));
    }

    HandleType Create(const UUID& uuid, std::unique_ptr<T>&& payload,
                      const Path* filename) {
        TL_RETURN_DEFAULT_IF_FALSE(uuid);
        TL_RETURN_DEFAULT_IF_FALSE(payload);
        return this->store(filename, uuid, std::move(payload));
    }

    virtual void Unload(HandleType handle) {
        auto uuid = handle.GetUUID();
        if (auto it = m_uuid_path_map.find(uuid); it != m_uuid_path_map.end()) {
            m_paths_uuid_map.erase(it->second);
            m_uuid_path_map.erase(it);
        }

        m_payloads.erase(uuid);
    }

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

    void Replace(UUID uuid, T&& payload) {
        if (auto it = m_payloads.find(uuid); it != m_payloads.end()) {
            *it->second = std::move(payload);
        }
    }

    void Reload(UUID uuid) {
        if (auto it = m_uuid_path_map.find(uuid); it != m_uuid_path_map.end()) {
            Load(it->second, true);
        }
    }

    void Reload(HandleType handle) {
        if (auto it = m_uuid_path_map.find(handle.GetUUID());
            it != m_uuid_path_map.end()) {
            Load(it->second);
        }
    }

    HandleType Replace(UUID uuid, const T& payload) {
        if (auto it = m_payloads.find(uuid); it != m_payloads.end()) {
            *it->second = payload;
            return HandleType{it->first, it->second.get(), this};
        }
        return {};
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

    void MakeEmbed(const UUID& uuid) {
        if constexpr (AssetSLInfo<T>::CanEmbed) {
            auto it = m_uuid_path_map.find(uuid);
            if (it != m_uuid_path_map.end()) {
                m_paths_uuid_map.erase(it->second);
            }
            m_uuid_path_map.erase(it);
        } else {
            LOGW("asset can't be embedded");
        }
    }

    void MakeEmbed(const HandleType handle) {
        TL_RETURN_IF_FALSE(handle);
        MakeEmbed(handle.GetUUID());
    }

    /** only change handle path, not save to file
     *
     * @warning only use for editor
     */
    void MakeExternal(const UUID& uuid, const Path& filename) {
        m_uuid_path_map[uuid] = filename;
        m_paths_uuid_map[filename] = uuid;
    }

    /** only change handle path, not save to file
     *
     * @warning only use for editor
     */
    void MakeExternal(HandleType handle, const Path& filename) {
        TL_RETURN_IF_FALSE(handle);

        const UUID& uuid = handle.GetUUID();
        if (const Path* old_filename = handle.GetFilename()) {
            m_paths_uuid_map.erase(*old_filename);
            m_uuid_path_map.erase(uuid);
        }

        m_uuid_path_map[uuid] = filename;
        m_paths_uuid_map[filename] = uuid;
    }

    void Clear() {
        m_payloads.clear();
        m_paths_uuid_map.clear();
        m_uuid_path_map.clear();
    }

protected:
    HandleType store(const Path* filename, UUID uuid,
                     std::unique_ptr<T>&& payload) {
        if (auto it = m_payloads.find(uuid); it != m_payloads.end()) {
            it->second.reset();
            it->second.swap(payload);
            payload.reset();
        } else {
            m_payloads.emplace(uuid, std::move(payload));
        }

        if (filename) {
            m_paths_uuid_map[*filename] = uuid;
            m_uuid_path_map[uuid] = *filename;
        }
        return HandleType{uuid, Find(uuid).Get(), this};
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

    HandleType Load(const Path& filename, bool force = false) override {
        if (auto handle = AssetManagerBase<T>::Find(filename);
            handle && !force) {
            return handle;
        }

        auto result = LoadAsset<T>(filename);
        TL_RETURN_DEFAULT_IF_FALSE_WITH_LOG(result, LOGE, "load asset {} failed",
                                            filename);
        return this->store(&filename, result.m_uuid,
                           std::move(result.m_payload));
    }
};
