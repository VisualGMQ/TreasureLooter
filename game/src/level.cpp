#include "level.hpp"

#include "SDL3/SDL.h"
#include "asset_manager.hpp"
#include "context.hpp"
#include "relationship.hpp"
#include "sprite.hpp"

Level::Level(LevelContentHandle level_content) {
    initByLevelContent(level_content);
}

Level::Level(const Path& filename) {
    auto handle =
        GAME_CONTEXT.m_assets_manager->GetManager<LevelContent>().Load(
            filename);
    initByLevelContent(handle);
}

Level::~Level() {
    for (auto entity : m_entities) {
        RemoveEntity(entity);
    }
    doRemoveEntities();
}

void Level::OnEnter() {
    if (!m_inited) {
        OnInit();
        m_inited = false;
    }
}

void Level::OnInit() { }

void Level::OnLogicUpdate(TimeType time) { }

void Level::OnRenderUpdate(TimeType time) { }

void Level::OnQuit() { }

void Level::PoseUpdate() {
    doRemoveEntities();
}

bool Level::IsInited() const {
    return m_inited;
}

Entity Level::Instantiate(PrefabHandle prefab) {
    Entity entity = GAME_CONTEXT.CreateEntity();
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

#ifdef TL_ENABLE_EDITOR
void Level::ReloadEntitiesFromPrefab(PrefabHandle prefab) {
    auto it = m_prefab_entity_map.find(prefab);
    if (it == m_prefab_entity_map.end()) {
        return;
    }

    for (auto entity : it->second) {
        auto transform = GAME_CONTEXT.m_transform_manager->Get(entity);

        EntityInstance instance;
        if (transform) {
            instance.m_transform = *transform;
        }
        instance.m_prefab = prefab;
        RemoveEntity(entity);
        registerEntity(entity, instance);
    }
}

void Level::ReloadEntitiesFromPrefab(UUID uuid) {
    auto prefab =
        GAME_CONTEXT.m_assets_manager->GetManager<Prefab>().Find(uuid);
    if (!prefab) {
        return;
    }
    ReloadEntitiesFromPrefab(prefab);
}
#endif

void Level::initRootEntity() {
    m_root_entity = GAME_CONTEXT.CreateEntity();
    GAME_CONTEXT.m_transform_manager->RegisterEntity(m_root_entity);
    GAME_CONTEXT.m_relationship_manager->RegisterEntity(m_root_entity);
}

void Level::registerEntity(Entity entity, const EntityInstance& instance) {
    createEntityByPrefab(
        entity,
        instance.m_transform ? &instance.m_transform.value() : nullptr,
        *instance.m_prefab);

#ifdef TL_ENABLE_EDITOR
    m_prefab_entity_map[instance.m_prefab].push_back(entity);
#endif
}

void Level::createEntityByPrefab(Entity entity, const Transform* transform,
                                 const Prefab& prefab) {
    if (prefab.m_sprite) {
        GAME_CONTEXT.m_sprite_manager->ReplaceComponent(
            entity, prefab.m_sprite.value());
    }
    if (transform || prefab.m_transform) {
        GAME_CONTEXT.m_transform_manager->ReplaceComponent(
            entity, transform ? *transform : prefab.m_transform.value());
    }
    if (prefab.m_tilemap) {
        GAME_CONTEXT.m_tilemap_component_manager->ReplaceComponent(
            entity, {entity, prefab.m_tilemap.value()});
    }
    if (prefab.m_animation) {
        GAME_CONTEXT.m_animation_player_manager->RegisterEntity(
            entity, prefab.m_animation.value());
    }
    if (prefab.m_cct) {
        GAME_CONTEXT.m_cct_manager->RegisterEntity(entity, entity,
                                                   prefab.m_cct.value());
        GAME_CONTEXT.m_cct_manager->Get(entity)->Teleport(
            prefab.m_transform->m_position);
    }
    if (prefab.m_trigger) {
        GAME_CONTEXT.m_trigger_component_manager->RegisterEntity(
            entity, entity, prefab.m_trigger.value());
    }

    if (prefab.m_motor_config) {
        switch (prefab.m_motor_config->m_type) {
            case MotorType::Unknown:
                LOGE("unknown motor type");
                break;
            case MotorType::Enemy:
                GAME_CONTEXT.m_motor_manager
                    ->RegisterEntityByDerive<EnemyMotorContext>(
                        entity, entity);
                break;
            case MotorType::Player:
                GAME_CONTEXT.m_motor_manager
                    ->RegisterEntityByDerive<PlayerMotorContext>(
                        entity, entity);
                break;
        }

        auto motor = GAME_CONTEXT.m_motor_manager->Get(entity);
        if (motor) {
            motor->Initialize(prefab.m_motor_config);
        }
    }

    if (!prefab.m_children.empty()) {
        GAME_CONTEXT.m_relationship_manager->RegisterEntity(entity);
        auto relationship = GAME_CONTEXT.m_relationship_manager->Get(entity);
        for (auto child : prefab.m_children) {
            EntityInstance instance;
            auto child_entity = GAME_CONTEXT.CreateEntity();
            instance.m_prefab = child;
            registerEntity(child_entity, instance);

            relationship->m_children.push_back(child_entity);
        }
    }

    if (auto motor = GAME_CONTEXT.m_motor_manager->Get(entity)) {
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
        auto entity = GAME_CONTEXT.CreateEntity();
        registerEntity(entity, instance);
        m_entities.insert(entity);

        auto relationship =
            GAME_CONTEXT.m_relationship_manager->Get(GetRootEntity());
        relationship->m_children.emplace_back(entity);
    }
}

void Level::doRemoveEntities() {
    std::vector<PrefabHandle> remove_prefabs;

    for (auto entity : m_pending_delete_entities) {
        GAME_CONTEXT.m_sprite_manager->RemoveEntity(entity);
        GAME_CONTEXT.m_transform_manager->RemoveEntity(entity);
        GAME_CONTEXT.m_relationship_manager->RemoveEntity(entity);
        GAME_CONTEXT.m_tilemap_component_manager->RemoveEntity(entity);
        GAME_CONTEXT.m_animation_player_manager->RemoveEntity(entity);
        GAME_CONTEXT.m_cct_manager->RemoveEntity(entity);

        m_entities.erase(entity);

#ifdef TL_ENABLE_EDITOR
        for (auto& [prefab, entities] : m_prefab_entity_map) {
            entities.erase(
                std::remove(entities.begin(), entities.end(), entity),
                entities.end());
            if (entities.empty()) {
                remove_prefabs.push_back(prefab);
            }
        }
#endif
    }

#ifdef TL_ENABLE_EDITOR
    for (auto prefab : remove_prefabs) {
        m_prefab_entity_map.erase(prefab);
    }
#endif

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
    if (level) {
        level->OnEnter();
    }
    m_level = level;
}

void LevelManager::UpdateLogic(TimeType t) {
    if (m_level) {
        m_level->OnLogicUpdate(t);
    }
}

void LevelManager::UpdateRender(TimeType t) {
    if (m_level) {
        m_level->OnRenderUpdate(t);
    }
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
