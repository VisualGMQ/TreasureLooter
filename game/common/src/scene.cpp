#include "common/scene.hpp"

#include "common/asset_manager.hpp"
#include "common/bind_point.hpp"
#include "common/cct.hpp"
#include "common/context.hpp"
#include "common/macros.hpp"
#include "common/relationship.hpp"
#include "common/tilemap_layer_collision_component.hpp"
#include "common/trigger.hpp"
#include "common/static_collision.hpp"
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
    Entity entity = COMMON_CONTEXT.CreateEntity();
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

void Scene::initByDescription(SceneDefinitionHandle level_content) {
    initRootEntity(level_content->m_script_path);
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
    auto relationship = COMMON_CONTEXT.m_relationship_manager->Get(entity);
    TL_RETURN_IF_NULL(relationship);

    Entity parent_entity = relationship->GetParent();
    TL_RETURN_IF_FALSE(parent_entity != null_entity);

    auto parent_relationship =
        COMMON_CONTEXT.m_relationship_manager->Get(parent_entity);
    TL_RETURN_IF_NULL(parent_relationship);
    parent_relationship->RemoveChild(entity);
}

void Scene::doRemoveEntityWithChildren(Entity entity) {
    TL_RETURN_IF_FALSE(entity != null_entity);

    auto relationship = COMMON_CONTEXT.m_relationship_manager->Get(entity);
    if (relationship) {
        for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
            doRemoveEntityWithChildren(relationship->Get(i));
        }
    }

    COMMON_CONTEXT.RemoveAllComponentsOnEntity(entity);
    
    m_entities.erase(entity);
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
