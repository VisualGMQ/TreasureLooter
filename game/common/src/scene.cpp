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

Entity Scene::Instantiate(PrefabHandle prefab, const Transform* transform) {
    Entity entity = COMMON_CONTEXT.CreateEntity();
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

void Scene::RemoveEntity(Entity entity) {
    COMMON_CONTEXT.RemoveEntity(entity);
}

void Scene::RemoveEntityFromInnerList(Entity entity) {
    m_entities.erase(entity);
}

bool Scene::HasEntity(Entity entity) const {
    return m_entities.count(entity) > 0;
}

Entity Scene::GetRootEntity() const {
    return m_root_entity;
}

void Scene::initEntities(SceneDefinitionHandle) {}

void Scene::initByDescription(SceneDefinitionHandle level_content) {
    initRootEntity(level_content->m_script_path);
    initEntities(level_content);
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

void SceneManager::RemoveEntity(Entity entity) {
    for (auto& [_, scene] : getAll()) {
        scene->RemoveEntityFromInnerList(entity);
    }
}

SceneHandle SceneManager::GetCurrentScene() const {
    return m_level;
}
