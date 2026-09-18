#include "server/scene.hpp"
#include "common/asset_manager.hpp"
#include "common/context.hpp"
#include "common/relationship.hpp"
#include "common/script/script.hpp"
#include "common/transform.hpp"
#include "server/context.hpp"

LogicEntity ServerScene::GetUIRootEntity() const {
    return null_entity;
}

void ServerScene::registerEntity(LogicEntity entity,
                                 const EntityInstance& instance) {
    COMMON_CONTEXT.AttachComponentsOnLogicEntity(entity, instance);
}

void ServerScene::initRootEntity(SceneDefinitionHandle scene_definition) {
    m_root_entity = COMMON_CONTEXT.CreateLogicEntity();
    m_entities.insert(m_root_entity);
    COMMON_CONTEXT.m_transform_manager->RegisterEntity(m_root_entity);
    COMMON_CONTEXT.m_relationship_manager->RegisterEntity(m_root_entity,
                                                          m_root_entity);

    auto handle =
        COMMON_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>().Load(
            scene_definition->m_server_script);
    if (handle) {
        COMMON_CONTEXT.m_script_component_manager->RegisterEntity(
            m_root_entity, m_root_entity, handle);
    }
}

void ServerScene::initEntities(SceneDefinitionHandle level_content) {
    auto root_relationship =
        COMMON_CONTEXT.m_relationship_manager->Get(m_root_entity);

    for (auto& instance : level_content->m_entities) {
        LogicEntity entity = Instantiate(instance.m_prefab);
        TL_CONTINUE_IF_FALSE(entity != null_entity);

        if (instance.m_transform) {
            auto* transform = COMMON_CONTEXT.m_transform_manager->Get(entity);
            if (transform) {
                *transform = *instance.m_transform;
            }
        }

        root_relationship->AddChild(entity);
    }
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
