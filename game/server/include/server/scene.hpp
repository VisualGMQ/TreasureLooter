#pragma once

#include "common/scene.hpp"

class ServerScene : public Scene {
public:
    using Scene::Scene;
    
protected:
    void registerEntity(Entity, const EntityInstance&) override;
    void initRootEntity(const Path& script_path) override;
};

class ServerSceneManager : public SceneManager {
public:
    SceneHandle Load(const Path& filename, bool force = false) override;
    SceneHandle Create(SceneDefinitionHandle) override;
};
