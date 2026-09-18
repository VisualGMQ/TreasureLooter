#pragma once

#include "common/scene.hpp"

class ClientScene : public Scene {
public:
    using Scene::Scene;
    
    void OnEnter() override;
    void OnQuit() override;

    [[nodiscard]] LogicEntity GetUIRootEntity() const override;
    
protected:
    void registerEntity(LogicEntity, const EntityInstance&) override;
    void initRootEntity(SceneDefinitionHandle) override;
    void initEntities(SceneDefinitionHandle level_content) override;
    
private:
    LogicEntity m_ui_root_entity{};
    EventListenerID m_window_resize_event_listener_id{};
};

class ClientSceneManager : public SceneManager {
public:
    SceneHandle Load(const Path& filename, bool force = false) override;
    SceneHandle Create(SceneDefinitionHandle) override;
};