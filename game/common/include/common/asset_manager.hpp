#pragma once

#include "common/animation.hpp"
#include "common/asset_manager_interface.hpp"
#include "common/context.hpp"
#include "common/scene.hpp"
#include "common/script/script.hpp"
#include "common/type_index.hpp"
#include "schema/gameplay_config.hpp"
#include <type_traits>

class IAssetsManager {
public:
    IAssetsManager() = default;
    virtual ~IAssetsManager() = default;

    GenericAssetManager<SceneDefinition>& GetSceneDefinitionAssetManager() {
        return ensureManager<GenericAssetManager<SceneDefinition>>(
            TypeIndexGenerator::Get<SceneDefinition>());
    }

    GenericAssetManager<LevelDefinition>& GetLevelDefinitionAssetManager() {
        return ensureManager<GenericAssetManager<LevelDefinition>>(
            TypeIndexGenerator::Get<LevelDefinition>());
    }

    template <typename T>
    auto& GetManager() {
        TypeIndex type_index = TypeIndexGenerator::Get<T>();
        return ensureManager<GenericAssetManager<T>>(type_index);
    }

protected:
    virtual ImageManagerBase& getImageManager(TypeIndex) = 0;
    virtual FontManagerBase& getFontManager(TypeIndex) = 0;

    template <typename T, typename... Args>
    T& ensureManager(TypeIndex index, Args&&... args) {
        if (index >= m_managers.size()) {
            m_managers.resize(index + 1);
        }

        if (!m_managers[index]) {
            m_managers[index] =
                std::make_unique<T>(std::forward<Args>(args)...);
        }

        return static_cast<T&>(*m_managers[index]);
    }

private:
    std::vector<std::unique_ptr<IAssetManager>> m_managers;
};

template <>
inline auto& IAssetsManager::GetManager<ScriptBinaryData>() {
    return *COMMON_CONTEXT.m_script_binary_data_manager;
}

template <>
inline auto& IAssetsManager::GetManager<Scene>() {
    return *COMMON_CONTEXT.m_scene_manager;
}

template <>
inline auto& IAssetsManager::GetManager<Animation>() {
    TypeIndex type_index = TypeIndexGenerator::Get<Animation>();
    return ensureManager<AnimationManager>(type_index);
}

template <>
inline auto& IAssetsManager::GetManager<Tilemap>() {
    TypeIndex type_index = TypeIndexGenerator::Get<Tilemap>();
    return ensureManager<TilemapManager>(type_index);
}

template <>
inline auto& IAssetsManager::GetManager<typename ImageHandle::underlying_type>() {
    return getImageManager(
        TypeIndexGenerator::Get<typename ImageHandle::underlying_type>());
}

template <>
inline auto& IAssetsManager::GetManager<typename FontHandle::underlying_type>() {
    return getFontManager(
        TypeIndexGenerator::Get<typename FontHandle::underlying_type>());
}
