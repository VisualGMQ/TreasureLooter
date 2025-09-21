#include "engine/level.hpp"

#include "SDL3/SDL.h"
#include "engine/asset_manager.hpp"
#include "engine/context.hpp"
#include "engine/relationship.hpp"
#include "engine/sprite.hpp"

Level::Level(LevelContentHandle level_content) {
    initByLevelContent(level_content);
}

Level::Level(const Path& filename) {
    auto handle =
        CURRENT_CONTEXT.m_assets_manager->GetManager<LevelContent>().Load(
            filename, true);
    initByLevelContent(handle);
}

Level::~Level() {
    OnQuit();
}

void Level::OnEnter() {
    if (!m_inited) {
        m_inited = false;
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

void Level::OnQuit() {
    CURRENT_CONTEXT.m_event_system->RemoveListener<SDL_WindowEvent>(
        m_window_resize_event_listener_id);

    for (auto entity : m_entities) {
        RemoveEntity(entity);
    }
    doRemoveEntities();
    m_entities.clear();
}

void Level::PoseUpdate() {
    doRemoveEntities();
}

bool Level::IsInited() const {
    return m_inited;
}

Entity Level::Instantiate(PrefabHandle prefab) {
    Entity entity = CURRENT_CONTEXT.CreateEntity();
    registerEntity(entity, {prefab->m_transform.value_or(Transform{}), prefab});
    m_entities.insert(entity);
    return entity;
}

void Level::RemoveEntity(Entity entity) {
    m_pending_delete_entities.push_back(entity);
}

Entity Level::GetRootEntity() const {
    return m_root_entity;
}

Entity Level::GetUIRootEntity() const {
    return m_ui_root_entity;
}

void Level::initRootEntity() {
    m_root_entity = CURRENT_CONTEXT.CreateEntity();
    m_entities.insert(m_root_entity);
    CURRENT_CONTEXT.m_transform_manager->RegisterEntity(m_root_entity);
    CURRENT_CONTEXT.m_relationship_manager->RegisterEntity(m_root_entity);

    m_ui_root_entity = CURRENT_CONTEXT.CreateEntity();
    m_entities.insert(m_ui_root_entity);
    CURRENT_CONTEXT.m_transform_manager->RegisterEntity(m_ui_root_entity);
    CURRENT_CONTEXT.m_relationship_manager->RegisterEntity(m_ui_root_entity);
    CURRENT_CONTEXT.m_ui_manager->RegisterEntity(m_ui_root_entity);
    UIWidget* ui = CURRENT_CONTEXT.m_ui_manager->Get(m_ui_root_entity);
    ui->m_anchor = UIAnchor::None;
    ui->m_panel = std::make_unique<UIPanelComponent>();
    Transform* transform =
        CURRENT_CONTEXT.m_transform_manager->Get(m_ui_root_entity);
    transform->m_size = CURRENT_CONTEXT.m_window->GetWindowSize();
}

void Level::registerEntity(Entity entity, const EntityInstance& instance) {
    createEntityByPrefab(
        entity, instance.m_transform ? &instance.m_transform.value() : nullptr,
        *instance.m_prefab);
}

void Level::createEntityByPrefab(Entity entity, const Transform* transform,
                                 const Prefab& prefab) {
    if (prefab.m_sprite) {
        CURRENT_CONTEXT.m_sprite_manager->ReplaceComponent(
            entity, prefab.m_sprite.value());
    }
    if (transform || prefab.m_transform) {
        CURRENT_CONTEXT.m_transform_manager->ReplaceComponent(
            entity, transform ? *transform : prefab.m_transform.value());
    }
    if (prefab.m_tilemap) {
        CURRENT_CONTEXT.m_tilemap_component_manager->ReplaceComponent(
            entity, {entity, prefab.m_tilemap.value()});
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
    if (prefab.m_ui) {
        CURRENT_CONTEXT.m_ui_manager->RegisterEntity(entity, prefab.m_ui);
    }

    if (prefab.m_motor_config) {
        switch (prefab.m_motor_config->m_type) {
            case MotorType::Unknown:
                LOGE("unknown motor type");
                break;
            case MotorType::Enemy:
                CURRENT_CONTEXT.m_motor_manager
                    ->RegisterEntityByDerive<EnemyMotorContext>(entity, entity);
                break;
            case MotorType::Player:
                CURRENT_CONTEXT.m_motor_manager
                    ->RegisterEntityByDerive<PlayerMotorContext>(entity,
                                                                 entity);
                break;
        }

        auto motor = CURRENT_CONTEXT.m_motor_manager->Get(entity);
        if (motor) {
            motor->Initialize(prefab.m_motor_config);
        }
    }

    if (!prefab.m_children.empty()) {
        CURRENT_CONTEXT.m_relationship_manager->RegisterEntity(entity);
        auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
        for (auto child : prefab.m_children) {
            Entity child_entity = Instantiate(child);
            relationship->m_children.push_back(child_entity);
        }
    }

    if (auto motor = CURRENT_CONTEXT.m_motor_manager->Get(entity)) {
        motor->Initialize(prefab.m_motor_config);
    }
}

void Level::initByLevelContent(LevelContentHandle level_content) {
    initRootEntity();

    for (auto& instance : level_content->m_entities) {
        if (!instance.m_prefab) {
            LOGW("prefab {} invalid", *instance.m_prefab.GetFilename());
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
        relationship->m_children.emplace_back(entity);
    }
}

void Level::doRemoveEntities() {
    std::vector<PrefabHandle> remove_prefabs;

    for (auto entity : m_pending_delete_entities) {
        CURRENT_CONTEXT.m_sprite_manager->RemoveEntity(entity);
        CURRENT_CONTEXT.m_transform_manager->RemoveEntity(entity);
        CURRENT_CONTEXT.m_relationship_manager->RemoveEntity(entity);
        CURRENT_CONTEXT.m_tilemap_component_manager->RemoveEntity(entity);
        CURRENT_CONTEXT.m_animation_player_manager->RemoveEntity(entity);
        CURRENT_CONTEXT.m_cct_manager->RemoveEntity(entity);
        CURRENT_CONTEXT.m_trigger_component_manager->RemoveEntity(entity);
        CURRENT_CONTEXT.m_motor_manager->RemoveEntity(entity);
        CURRENT_CONTEXT.m_bind_point_component_manager->RemoveEntity(entity);

        m_entities.erase(entity);
    }

    m_pending_delete_entities.clear();
}

AssetManagerBase<Level>::HandleType LevelManager::Load(const Path& filename,
                                                       bool force) {
    if (auto handle = Find(filename); handle && !force) {
        return handle;
    }
    return store(&filename, UUID::CreateV4(),
                 std::make_unique<Level>(filename));
}

void LevelManager::Switch(LevelHandle level) {
    if (m_level) {
        m_level->OnQuit();
    }
    Unload(m_level);
    if (level) {
        level->OnEnter();
    }
    m_level = level;
}

void LevelManager::PoseUpdate() {
    if (!m_level) {
        return;
    }
    m_level->PoseUpdate();
}

LevelHandle LevelManager::GetCurrentLevel() const {
    return m_level;
}
