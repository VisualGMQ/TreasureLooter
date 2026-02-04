#pragma once

#include "context.hpp"
#include "animation.hpp"
#include "asset_manager_interface.hpp"
#include "image.hpp"
#include "level.hpp"
#include "prefab_manager.hpp"
#include "text.hpp"
#include "tilemap.hpp"

class AssetsManager {
public:
    AssetsManager();
    ~AssetsManager();

    template <typename T>
    auto& GetManager() {
        TypeIndex index = TypeIndexGenerator::Get<T>();
        if constexpr (std::is_same_v<T, Image>) {
            return ensureManager<ImageManager>(index, *CURRENT_CONTEXT.m_renderer);
        } else if constexpr (std::is_same_v<T, Tilemap>) {
            return ensureManager<TilemapManager>(index);
        } else if constexpr (std::is_same_v<T, Animation>) {
            return ensureManager<AnimationManager>(index);
        } else if constexpr (std::is_same_v<T, Level>) {
            return *CURRENT_CONTEXT.m_level_manager;
        } else if constexpr (std::is_same_v<T, Prefab>) {
            return ensureManager<PrefabManager>(index);
        } else if constexpr (std::is_same_v<T, Font>) {
            return ensureManager<FontManager>(index);
        } else if constexpr (std::is_same_v<T, ScriptBinaryData>) {
            return ensureManager<ScriptBinaryDataManager>(index);
        } else {
            return ensureManager<GenericAssetManager<T>>(index);
        }
    }

private:
    std::vector<std::unique_ptr<IAssetManager>> m_managers;

    // script binary data manager must release at least, so extra declare it
    std::unique_ptr<ScriptBinaryDataManager> m_script_binary_data_manager;

    template <typename T, typename... Args>
    T& ensureManager(TypeIndex index, Args&&... args) {
        if (index >= m_managers.size()) {
            m_managers.resize(index + 1);
        }

        if (!m_managers[index]) {
            m_managers[index] = std::make_unique<T>(std::forward<Args>(args)...);
        }

        return static_cast<T&>(*m_managers[index]);
    }
};
