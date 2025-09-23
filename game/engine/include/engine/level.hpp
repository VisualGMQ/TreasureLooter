#pragma once
#include "animation.hpp"
#include "event.hpp"
#include "image.hpp"
#include "schema/level_content.hpp"
#include "timer.hpp"

#include <unordered_set>

class Level {
public:
    friend class LevelManager;

    Level() = default;

    explicit Level(LevelContentHandle);

    explicit Level(const Path& filename);

    ~Level();

    void Initialize();
    void OnEnter();

    void OnQuit();

    void PoseUpdate();

    [[nodiscard]] bool IsInited() const;

    /**
     * instantiate entity but don't put into level scene
     */
    [[nodiscard]] Entity Instantiate(PrefabHandle);

    void RemoveEntity(Entity);

    [[nodiscard]] Entity GetRootEntity() const;
    [[nodiscard]] Entity GetUIRootEntity() const;

private:
    bool m_visible = false;
    bool m_inited = false;
    LevelContentHandle m_pending_init_content;

    Entity m_root_entity{};
    Entity m_ui_root_entity{};
    std::unordered_set<Entity> m_entities;

    std::vector<Entity> m_pending_delete_entities;
    EventListenerID m_window_resize_event_listener_id{};

    void initRootEntity();

    void registerEntity(Entity, const EntityInstance&);

    void createEntityByPrefab(Entity entity, const Transform*,
                              const Prefab& prefab);

    void initByLevelContent(LevelContentHandle);

    void doRemoveEntities();
};

using LevelHandle = Handle<Level>;

class LevelManager final : public AssetManagerBase<Level> {
public:
    HandleType Load(const Path& filename, bool force = false) override;

    void Switch(LevelHandle);

    void PoseUpdate();

    LevelHandle GetCurrentLevel() const;

private:
    LevelHandle m_level;
};
