#pragma once
#include "common/animation.hpp"
#include "common/event.hpp"
#include "common/image.hpp"
#include "schema/gameplay_config.hpp"
#include "schema/scene_definition.hpp"

#include <functional>
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

    [[nodiscard]] bool IsInited() const;

    /**
     * instantiate entity but don't put into level scene
     */
    LogicEntity Instantiate(PrefabHandle, const Transform* = nullptr);

    void RemoveEntity(LogicEntity);
    void RemoveEntityFromInnerList(LogicEntity);

    [[nodiscard]] bool HasEntity(LogicEntity) const;

    [[nodiscard]] LogicEntity GetRootEntity() const;
    [[nodiscard]] virtual LogicEntity GetUIRootEntity() const = 0;

    [[nodiscard]] const std::unordered_set<LogicEntity>& GetAllEntities() const;

protected:
    virtual void registerEntity(LogicEntity, const EntityInstance&) = 0;
    virtual void initRootEntity(SceneDefinitionHandle) = 0;
    virtual void initEntities(SceneDefinitionHandle level_content) = 0;

    LogicEntity m_root_entity{};

    std::unordered_set<LogicEntity> m_entities;

private:
    bool m_visible = false;
    bool m_inited = false;
    SceneDefinitionHandle m_pending_init_description;

    void initByDescription(SceneDefinitionHandle);
};

using SceneHandle = Handle<Scene>;

class SceneManager : public AssetManagerBase<Scene> {
public:
    using AssetManagerBase<Scene>::Load;
    
    virtual SceneHandle Create(SceneDefinitionHandle) = 0;

    void Switch(SceneHandle level);

    void SwitchImmediate(SceneHandle level);

    void RemoveEntity(LogicEntity);

    [[nodiscard]] SceneHandle GetCurrentScene() const;

    void Update();

private:
    SceneHandle m_level;
    SceneHandle m_pending_level;
    std::function<void()> m_pending_switch_callback;
};
