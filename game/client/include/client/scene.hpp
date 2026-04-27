#pragma once

#include "common/scene.hpp"

class ClientScene : public Scene {
public:
    using Scene::Scene;
    
    void OnEnter() override;
    void OnQuit() override;
    
protected:
    void registerEntity(Entity, const EntityInstance&) override;
    void initRootEntity(const Path& script_path) override;
    
private:
    EventListenerID m_window_resize_event_listener_id{};
};

class ClientSceneManager : public SceneManager {
public:
    SceneHandle Load(const Path& filename, bool force = false) override;
    SceneHandle Create(SceneDefinitionHandle) override;
};