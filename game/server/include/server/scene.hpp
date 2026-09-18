#pragma once

#include "common/scene.hpp"

class ServerScene : public Scene {
public:
    using Scene::Scene;

    [[nodiscard]] LogicEntity GetUIRootEntity() const override;

protected:
    void registerEntity(LogicEntity, const EntityInstance&) override;
    void initRootEntity(SceneDefinitionHandle) override;
    void initEntities(SceneDefinitionHandle level_content) override;
};

class ServerSceneManager : public SceneManager {
public:
    SceneHandle Load(const Path& filename, bool force = false) override;
    SceneHandle Create(SceneDefinitionHandle) override;
};
