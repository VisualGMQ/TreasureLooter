#include "engine/scene.hpp"

#include "SDL3/SDL.h"
#include "engine/asset_manager.hpp"
#include "engine/bind_point.hpp"
#include "engine/cct.hpp"
#include "engine/context.hpp"
#include "engine/draw_order.hpp"
#include "engine/macros.hpp"
#include "engine/relationship.hpp"
#include "engine/sprite.hpp"
#include "engine/tilemap.hpp"
#include "engine/trigger.hpp"
#include "engine/ui.hpp"
#include "schema/scene_definition.hpp"

Scene::Scene(SceneDefinitionHandle level_content) {
    m_pending_init_description = level_content;
}

Scene::Scene(const Path& filename) {
    auto handle =
        CURRENT_CONTEXT.m_assets_manager->GetManager<SceneDefinition>().Load(
            filename, true);
    m_pending_init_description = handle;
}

Scene::~Scene() {
    OnQuit();
}

void Scene::Initialize() {
    initByDescription(m_pending_init_description);
    m_pending_init_description = {};
}

void Scene::OnEnter() {
    if (!m_inited) {
        Initialize();
        m_inited = true;
    }
    Transform* transform =
        CURRENT_CONTEXT.m_transform_manager->Get(GetUIRootEntity());
    transform->m_size = CURRENT_CONTEXT.m_window->GetWindowSize();

    m_window_resize_event_listener_id =
        CURRENT_CONTEXT.m_event_system->AddListener<SDL_WindowEvent>(
            [this](EventListenerID, const SDL_WindowEvent& event) {
                if (event.type != SDL_EVENT_WINDOW_RESIZED &&
                    event.type != SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED &&
                    event.type != SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED) {
                    return;
                }
                Entity entity = this->GetUIRootEntity();
                Transform* transform =
                    CURRENT_CONTEXT.m_transform_manager->Get(entity);
                if (!(event.data1 == 0 && event.data2 == 0)) {
                    transform->m_size.w = event.data1;
                    transform->m_size.h = event.data2;
                }
            });
}

void Scene::OnQuit() {
    CURRENT_CONTEXT.m_event_system->RemoveListener<SDL_WindowEvent>(
        m_window_resize_event_listener_id);

    for (auto entity : m_entities) {
        RemoveEntity(entity);
    }
    doRemoveEntities();
    m_entities.clear();
}

void Scene::PoseUpdate() {
    doRemoveEntities();
}

bool Scene::IsInited() const {
    return m_inited;
}

Entity Scene::Instantiate(PrefabHandle prefab) {
    Entity entity = CURRENT_CONTEXT.CreateEntity();
    registerEntity(entity, {prefab->m_transform.value_or(Transform{}), prefab});
    m_entities.insert(entity);
    return entity;
}

void Scene::RemoveEntity(Entity entity) {
    m_pending_delete_entities.push_back(entity);
}

Entity Scene::GetRootEntity() const {
    return m_root_entity;
}

Entity Scene::GetUIRootEntity() const {
    return m_ui_root_entity;
}

void Scene::initRootEntity(const Path& script_path) {
    m_root_entity = CURRENT_CONTEXT.CreateEntity();
    m_entities.insert(m_root_entity);
    CURRENT_CONTEXT.m_transform_manager->RegisterEntity(m_root_entity);
    CURRENT_CONTEXT.m_relationship_manager->RegisterEntity(m_root_entity,
                                                           m_root_entity);
    CURRENT_CONTEXT.m_draw_order_manager->RegisterEntity(m_root_entity);

    m_ui_root_entity = CURRENT_CONTEXT.CreateEntity();
    m_entities.insert(m_ui_root_entity);
    CURRENT_CONTEXT.m_transform_manager->RegisterEntity(m_ui_root_entity);
    CURRENT_CONTEXT.m_relationship_manager->RegisterEntity(m_ui_root_entity,
                                                           m_ui_root_entity);
    CURRENT_CONTEXT.m_ui_manager->RegisterEntity(m_ui_root_entity);
    UIWidget* ui = CURRENT_CONTEXT.m_ui_manager->Get(m_ui_root_entity);
    ui->m_anchor = UIAnchor::None;
    ui->m_panel = std::make_unique<UIPanelComponent>();
    Transform* transform =
        CURRENT_CONTEXT.m_transform_manager->Get(m_ui_root_entity);
    transform->m_size = CURRENT_CONTEXT.m_window->GetWindowSize();

    if (!script_path.empty()) {
        auto handle =
            CURRENT_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>()
                .Load(script_path);
        CURRENT_CONTEXT.m_script_component_manager->RegisterEntity(
            m_root_entity, m_root_entity, handle);
    }
}

void Scene::registerEntity(Entity entity, const EntityInstance& instance) {
    createEntityByPrefab(
        entity, instance.m_transform ? &instance.m_transform.value() : nullptr,
        *instance.m_prefab);
}

void Scene::createEntityByPrefab(Entity entity, const Transform* transform,
                                 const Prefab& prefab) {
    if (prefab.m_sprite) {
        CURRENT_CONTEXT.m_sprite_manager->RegisterEntity(
            entity, prefab.m_sprite.value());
    }
    if (transform || prefab.m_transform) {
        CURRENT_CONTEXT.m_transform_manager->RegisterEntity(
            entity, transform ? *transform : prefab.m_transform.value());
    }
    if (prefab.m_tilemap_layer) {
        CURRENT_CONTEXT.m_tilemap_layer_component_manager->RegisterEntity(
            entity,
            TilemapLayerComponent{entity, prefab.m_tilemap_layer.value()});
    }
    if (prefab.m_draw_order) {
        CURRENT_CONTEXT.m_draw_order_manager->RegisterEntity(
            entity, prefab.m_draw_order.value());
    }
    if (prefab.m_animation) {
        CURRENT_CONTEXT.m_animation_player_manager->RegisterEntity(
            entity, prefab.m_animation.value());
    }
    if (prefab.m_cct) {
        CURRENT_CONTEXT.m_cct_manager->RegisterEntity(entity, entity,
                                                      prefab.m_cct.value());
        CURRENT_CONTEXT.m_cct_manager->Get(entity)->Teleport(
            prefab.m_transform->m_position);
    }
    if (prefab.m_trigger) {
        CURRENT_CONTEXT.m_trigger_component_manager->RegisterEntity(
            entity, entity, prefab.m_trigger.value());
    }
    if (!prefab.m_bind_points.empty()) {
        CURRENT_CONTEXT.m_bind_point_component_manager->RegisterEntity(
            entity, prefab.m_bind_points);
    }
    if (prefab.m_ui) {
        CURRENT_CONTEXT.m_ui_manager->RegisterEntity(entity, prefab.m_ui);
    }
    if (!prefab.m_script.empty()) {
        auto& mgr =
            CURRENT_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>();
        ScriptBinaryDataHandle handle = mgr.Load(prefab.m_script);
        CURRENT_CONTEXT.m_script_component_manager->RegisterEntity(
            entity, entity, handle);
    }

    // every entity has relationship
    CURRENT_CONTEXT.m_relationship_manager->RegisterEntity(entity, entity);
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    for (auto child : prefab.m_children) {
        Entity child_entity = Instantiate(child);
        relationship->AddChild(child_entity);
    }
}

void Scene::initByDescription(SceneDefinitionHandle level_content) {
    initRootEntity(level_content->m_script_path);

    for (auto& instance : level_content->m_entities) {
        if (!instance.m_prefab) {
            auto filename = instance.m_prefab.GetFilename();
            LOGW("prefab {} invalid", filename ? *filename : "<embed>");
            continue;
        }
        auto entity = CURRENT_CONTEXT.CreateEntity();
        registerEntity(entity, instance);
        m_entities.insert(entity);

        Relationship* relationship =
            CURRENT_CONTEXT.m_relationship_manager->Get(GetRootEntity());
        if (CURRENT_CONTEXT.m_ui_manager->Has(entity)) {
            relationship =
                CURRENT_CONTEXT.m_relationship_manager->Get(GetUIRootEntity());
        }
        relationship->AddChild(entity);
    }
}

void Scene::doRemoveEntities() {
    std::vector<PrefabHandle> remove_prefabs;

    for (auto entity : m_pending_delete_entities) {
        doRemoveEntityFromParent(entity);
        doRemoveEntityWithChildren(entity);
    }

    m_pending_delete_entities.clear();
}

void Scene::doRemoveEntityFromParent(Entity entity) {
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    TL_RETURN_IF_NULL(relationship);

    Entity parent_entity = relationship->GetParent();
    TL_RETURN_IF_FALSE(parent_entity != null_entity);

    auto parent_relationship = CURRENT_CONTEXT.m_relationship_manager->Get(parent_entity);
    TL_RETURN_IF_NULL(parent_relationship);
    parent_relationship->RemoveChild(entity);
}

void Scene::doRemoveEntityWithChildren(Entity entity) {
    TL_RETURN_IF_FALSE(entity != null_entity);

    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    if (relationship) {
        for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
            doRemoveEntityWithChildren(relationship->Get(i));
        }
    }

    CURRENT_CONTEXT.m_sprite_manager->RemoveEntity(entity);
    CURRENT_CONTEXT.m_transform_manager->RemoveEntity(entity);
    CURRENT_CONTEXT.m_relationship_manager->RemoveEntity(entity);
    CURRENT_CONTEXT.m_tilemap_layer_component_manager->RemoveEntity(entity);
    CURRENT_CONTEXT.m_animation_player_manager->RemoveEntity(entity);
    CURRENT_CONTEXT.m_cct_manager->RemoveEntity(entity);
    CURRENT_CONTEXT.m_trigger_component_manager->RemoveEntity(entity);
    CURRENT_CONTEXT.m_bind_point_component_manager->RemoveEntity(entity);
    CURRENT_CONTEXT.m_ui_manager->RemoveEntity(entity);
    CURRENT_CONTEXT.m_script_component_manager->RemoveEntity(entity);
    CURRENT_CONTEXT.m_draw_order_manager->RemoveEntity(entity);

    m_entities.erase(entity);
}

AssetManagerBase<Scene>::HandleType SceneManager::Load(const Path& filename,
                                                       bool force) {
    if (auto handle = Find(filename); handle && !force) {
        return handle;
    }
    return store(&filename, UUID::CreateV4(),
                 std::make_unique<Scene>(filename));
}

SceneHandle SceneManager::Create(SceneDefinitionHandle handle) {
    return store(nullptr, UUID::CreateV4(), std::make_unique<Scene>(handle));
}

void SceneManager::Switch(SceneHandle level) {
    if (m_level) {
        m_level->OnQuit();
    }
    Unload(m_level);
    m_level = level;
    if (level) {
        level->OnEnter();
    }
}

void SceneManager::PoseUpdate() {
    if (!m_level) {
        return;
    }
    m_level->PoseUpdate();
}

SceneHandle SceneManager::GetCurrentScene() const {
    return m_level;
}
