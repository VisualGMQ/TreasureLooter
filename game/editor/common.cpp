#include "common.hpp"

#include "context.hpp"
#include "engine/asset_manager.hpp"
#include "engine/relationship.hpp"
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

void SaveEntity(Entity entity) {
    PrefabHandle prefab_handle = EDITOR_CONTEXT.m_entity_prefab_component->Get(entity);
    if (!prefab_handle) {
        LOGE("entity not has related prefab");
        return;
    }

    if (auto transform = CURRENT_CONTEXT.m_transform_manager->Get(entity)) {
        prefab_handle->m_transform = *transform;
    }

    if (auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity)) {
        for (auto child : relationship->m_children) {
            PrefabHandle child_prefab = EDITOR_CONTEXT.m_entity_prefab_component->Get(child);
            if (!child_prefab) {
                LOGE("entity not has related prefab");
                continue;
            }
            prefab_handle->m_children.push_back(child_prefab);
        }
    }

    if (auto sprite = CURRENT_CONTEXT.m_sprite_manager->Get(entity)) {
        prefab_handle->m_sprite = *sprite;
    }

    if (auto animator = CURRENT_CONTEXT.m_animation_player_manager->Get(entity)) {
        AnimationPlayerDefinition create_info;
        create_info.m_auto_play = animator->IsAutoPlayEnabled();
        create_info.m_loop = animator->GetLoopCount();
        create_info.m_rate = animator->GetRate();
        create_info.m_animation = animator->GetAnimation();
        
        prefab_handle->m_animation  = create_info;
    }

    if (auto cct = CURRENT_CONTEXT.m_cct_manager->Get(entity)) {
        CCTDefinition prefab_cct;
        prefab_cct.m_min_disp = cct->GetMinDisp();
        prefab_cct.m_skin = cct->GetSkin();
        
        // TODO: 
        prefab_cct.m_physics_actor = prefab_handle->m_cct->m_physics_actor;
        
        prefab_handle->m_cct = prefab_cct; 
    }

    if (auto trigger = CURRENT_CONTEXT.m_trigger_component_manager->Get(entity)) {
        TriggerDefinition trigger_info;
        trigger_info.m_event_type = trigger->GetEventType();
        trigger_info.m_trig_every_frame_when_touch = trigger->IsTriggerEveryFrameWhenTouch();
        trigger_info.m_physics_actor = prefab_handle->m_cct->m_physics_actor;
        prefab_handle->m_trigger = trigger_info;
    }

    if (auto motor = CURRENT_CONTEXT.m_gameplay_config_manager->Get(entity)) {
        GameplayConfig config;
        *prefab_handle->m_gameplay_config.Get() = config;
    }

    if (auto tilemap = CURRENT_CONTEXT.m_tilemap_component_manager->Get(entity)) {
        TilemapDefinition tilemap_def;
        tilemap_def.m_tilemap = tilemap->GetHandle();
        tilemap_def.m_position = {}; // TODO
        prefab_handle->m_tilemap = tilemap_def;
    }

    SaveVariantAsset(prefab_handle);
}
