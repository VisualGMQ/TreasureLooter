#pragma once
#include "engine/animation.hpp"
#include "engine/event.hpp"
#include "engine/image.hpp"
#include "schema/scene_definition.hpp"

#include <unordered_set>

class Scene {
public:
    friend class SceneManager;

    Scene() = default;

    explicit Scene(SceneDefinitionHandle);

    explicit Scene(const Path& filename);

    ~Scene();

    void Initialize();
    void OnEnter();

    void OnQuit();

    void PoseUpdate();

    bool IsInited() const;

    /**
     * instantiate entity but don't put into level scene
     */
    Entity Instantiate(PrefabHandle);

    void RemoveEntity(Entity);

    Entity GetRootEntity() const;
    Entity GetUIRootEntity() const;

private:
    bool m_visible = false;
    bool m_inited = false;
    SceneDefinitionHandle m_pending_init_description;

    Entity m_root_entity{};
    Entity m_ui_root_entity{};
    std::unordered_set<Entity> m_entities;

    std::vector<Entity> m_pending_delete_entities;
    EventListenerID m_window_resize_event_listener_id{};

    void initRootEntity(const Path& script_path);

    void registerEntity(Entity, const EntityInstance&);

    void createEntityByPrefab(Entity entity, const Transform*,
                              const Prefab& prefab);

    void initByDescription(SceneDefinitionHandle);

    void doRemoveEntities();
    void doRemoveEntityWithChildren(Entity);
};

using SceneHandle = Handle<Scene>;

class SceneManager final : public AssetManagerBase<Scene> {
public:
    HandleType Load(const Path& filename, bool force = false) override;
    SceneHandle Create(SceneDefinitionHandle);

    void Switch(SceneHandle);

    void PoseUpdate();

    SceneHandle GetCurrentScene() const;

private:
    SceneHandle m_level;
};
