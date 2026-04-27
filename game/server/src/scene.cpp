#include "server/scene.hpp"

void ServerScene::registerEntity(Entity, const EntityInstance&) {
    // TODO: not finish
}
void ServerScene::initRootEntity(const Path& script_path) {
    // TODO: not finish
}

SceneHandle ServerSceneManager::Load(const Path& filename, bool force) {
    if (auto handle = Find(filename); handle && !force) {
        return handle;
    }
    return store(&filename, UUID::CreateV4(),
                 std::make_unique<ServerScene>(filename));
}

SceneHandle ServerSceneManager::Create(SceneDefinitionHandle handle) {
    return store(nullptr, UUID::CreateV4(), std::make_unique<ServerScene>(handle));
}
