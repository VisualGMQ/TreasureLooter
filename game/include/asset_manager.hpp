#pragma once
#include "handle.hpp"

#include "log.hpp"
#include "path.hpp"
#include "schema/serialize/input.hpp"
#include "uuid.hpp"

#include <memory>
#include <unordered_map>

template <typename T>
class AssetManager {
public:
    using HandleType = Handle<T>;

    virtual ~AssetManager() = default;

    virtual HandleType Load(const Path& filename) = 0;

    HandleType Find(const Path& filename) {
        if (auto it = m_paths.find(filename); it != m_paths.end()) {
            if (auto it2 = m_payloads.find(it->second);
                it2 != m_payloads.end()) {
                return {it2->first, it2->second.get()};
            }
        }
        return nullptr;
    }

    bool IsExists(const Path& filename) const {
        return m_payloads.find(filename) != m_payloads.end();
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
            auto r = m_paths.emplace(*filename, uuid);
            if (!r.second) {
                LOGE("emplace filename failed: {}", *filename);
                return nullptr;
            }
        }
        return HandleType{uuid, result.first->second.get()};
    }

private:
    std::unordered_map<UUID, std::unique_ptr<T>> m_payloads;
    std::unordered_map<Path, UUID> m_paths;
};

template <typename T>
class GenericAssetManager : public AssetManager<T> {
public:
    using HandleType = typename AssetManager<T>::HandleType;

    HandleType Load(const Path& filename) override {
        auto result = LoadAsset<T>(filename);
        return store(&filename, result.m_uuid,
                     std::make_unique<T>(std::move(result.m_value)));
    }
};