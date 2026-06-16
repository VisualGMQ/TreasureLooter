#include "server/scene.hpp"
#include "common/asset_manager.hpp"
#include "common/context.hpp"
#include "common/relationship.hpp"
#include "common/script/script.hpp"
#include "common/transform.hpp"
#include "server/context.hpp"

Entity ServerScene::GetUIRootEntity() const {
    return null_entity;
}

void ServerScene::registerEntity(Entity entity,
                                 const EntityInstance& instance) {
    COMMON_CONTEXT.AttachComponentsOnEntity(entity, instance);
}

void ServerScene::initRootEntity(const Path& script_path) {
    m_root_entity = COMMON_CONTEXT.CreateEntity();
    m_entities.insert(m_root_entity);
    COMMON_CONTEXT.m_transform_manager->RegisterEntity(m_root_entity);
    COMMON_CONTEXT.m_relationship_manager->RegisterEntity(m_root_entity,
                                                          m_root_entity);
}

SceneHandle ServerSceneManager::Load(const Path& filename, bool force) {
    if (auto handle = Find(filename); handle && !force) {
        return handle;
    }
    return store(&filename, UUIDv4::CreateV4(),
                 std::make_unique<ServerScene>(filename));
}

SceneHandle ServerSceneManager::Create(SceneDefinitionHandle handle) {
    return store(nullptr, UUIDv4::CreateV4(),
                 std::make_unique<ServerScene>(handle));
}

void ServerSceneManager::Switch(SceneHandle scene) {
    SceneManager::Switch(scene);
}
