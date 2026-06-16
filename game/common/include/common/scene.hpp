#pragma once
#include "common/animation.hpp"
#include "common/event.hpp"
#include "common/image.hpp"
#include "schema/scene_definition.hpp"

#include <unordered_set>

class Scene {
public:
    friend class SceneManager;

    Scene() = default;

    explicit Scene(SceneDefinitionHandle);

    explicit Scene(const Path& filename);

    virtual ~Scene();

    void Initialize();
    virtual void OnEnter();

    virtual void OnQuit();

    void PoseUpdate();

    bool IsInited() const;

    /**
     * instantiate entity but don't put into level scene
     */
    Entity Instantiate(PrefabHandle);

    void RemoveEntity(Entity);

    Entity GetRootEntity() const;
    virtual Entity GetUIRootEntity() const = 0;

protected:
    virtual void registerEntity(Entity, const EntityInstance&) = 0;
    virtual void initRootEntity(const Path& script_path) = 0;
    virtual void initEntities(SceneDefinitionHandle level_content);

    Entity m_root_entity{};

    std::unordered_set<Entity> m_entities;

private:
    bool m_visible = false;
    bool m_inited = false;
    SceneDefinitionHandle m_pending_init_description;

    std::vector<Entity> m_pending_delete_entities;

    void initByDescription(SceneDefinitionHandle);

    void doRemoveEntities();
    void doRemoveEntityWithChildren(Entity);
    void doRemoveEntityFromParent(Entity);
};

using SceneHandle = Handle<Scene>;

class SceneManager : public AssetManagerBase<Scene> {
public:
    using AssetManagerBase<Scene>::Load;
    
    virtual SceneHandle Create(SceneDefinitionHandle) = 0;

    void Switch(SceneHandle);

    void PoseUpdate();

    SceneHandle GetCurrentScene() const;

private:
    SceneHandle m_level;
};
