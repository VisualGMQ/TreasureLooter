#pragma once
#include "animation.hpp"
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

    void OnEnter();

    void OnQuit();

    void PoseUpdate();

    bool IsInited() const;

    Entity Instantiate(PrefabHandle);

    void RemoveEntity(Entity);

    Entity GetRootEntity() const;

#ifdef TL_ENABLE_EDITOR
    void ReloadEntitiesFromPrefab(PrefabHandle);

    void ReloadEntitiesFromPrefab(UUID);
#endif

private:
    bool m_visible = false;
    bool m_inited = false;

    Entity m_root_entity{};
    std::unordered_set<Entity> m_entities;

    std::vector<Entity> m_pending_delete_entities;

#ifdef TL_ENABLE_EDITOR
    std::unordered_map<PrefabHandle, std::vector<Entity> > m_prefab_entity_map;
#endif

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
