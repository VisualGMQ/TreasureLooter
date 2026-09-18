#include "common/scene.hpp"

#include "common/asset_manager.hpp"
#include "common/bind_point.hpp"
#include "common/cct.hpp"
#include "common/context.hpp"
#include "common/event.hpp"
#include "common/macros.hpp"
#include "common/relationship.hpp"
#include "common/static_collision.hpp"
#include "common/tilemap_layer_collision_component.hpp"
#include "common/trigger.hpp"
#include "schema/scene_definition.hpp"

Scene::Scene(SceneDefinitionHandle level_content) {
    m_pending_init_description = level_content;
}

Scene::Scene(const Path& filename) {
    auto handle =
        COMMON_CONTEXT.m_assets_manager->GetManager<SceneDefinition>().Load(
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
}

void Scene::OnQuit() {
    for (auto entity : m_entities) {
        COMMON_CONTEXT.RemoveEntity(entity);
    }
    m_entities.clear();
}

bool Scene::IsInited() const {
    return m_inited;
}

LogicEntity Scene::Instantiate(PrefabHandle prefab, const Transform* transform) {
    LogicEntity entity = COMMON_CONTEXT.CreateLogicEntity();
    Transform trans;
    if (transform) {
        trans = *transform;
    } else {
        trans = prefab->m_transform.value_or(Transform{});
    }
    registerEntity(entity, {trans, prefab});
    m_entities.insert(entity);
    return entity;
}

void Scene::RemoveEntity(LogicEntity entity) {
    COMMON_CONTEXT.RemoveEntity(entity);
}

void Scene::RemoveEntityFromInnerList(LogicEntity entity) {
    m_entities.erase(entity);
}

bool Scene::HasEntity(LogicEntity entity) const {
    return m_entities.count(entity) > 0;
}

LogicEntity Scene::GetRootEntity() const {
    return m_root_entity;
}

const std::unordered_set<LogicEntity>& Scene::GetAllEntities() const {
    return m_entities;
}

void Scene::initByDescription(SceneDefinitionHandle level_content) {
    initRootEntity(level_content);
    initEntities(level_content);
}

void SceneManager::Switch(SceneHandle level) {
    m_pending_level = level;
}

void SceneManager::SwitchImmediate(SceneHandle level) {
    if (m_level) {
        m_level->OnQuit();
    }
    Unload(m_level);
    m_level = level;
    if (level) {
        level->OnEnter();
    }
}

void SceneManager::RemoveEntity(LogicEntity entity) {
    for (auto& [_, scene] : getAll()) {
        scene->RemoveEntityFromInnerList(entity);
    }
}

SceneHandle SceneManager::GetCurrentScene() const {
    return m_level;
}

void SceneManager::Update() {
    TL_RETURN_IF_FALSE(m_pending_level);
    SwitchImmediate(m_pending_level);
    m_pending_level = nullptr;
}
