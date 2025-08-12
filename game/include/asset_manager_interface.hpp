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

    virtual HandleType Load(const Path& filename, bool force = false) = 0;

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
        if (auto it = m_uuid_path_map.find(handle.GetUUID()); it != m_uuid_path_map.end()) {
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
        if (auto handle = AssetManagerBase<T>::Find(filename); handle && !force) {
            return handle;
        }

        auto result = LoadAsset<T>(filename);
        return this->store(&filename, result.m_uuid,
                           std::make_unique<T>(std::move(result.m_payload)));
    }

    HandleType Create() {
        return this->store(nullptr, UUID::CreateV4(), std::make_unique<T>());
    }

    HandleType Create(const T& value) {
        return this->store(nullptr, UUID::CreateV4(),
                           std::make_unique<T>(value));
    }
};