#pragma once
#include "animation.hpp"
#include "entity_logic.hpp"
#include "image.hpp"
#include "schema/level.hpp"
#include "timer.hpp"

#include <unordered_set>

class Level {
public:
    Level() = default;
    explicit Level(LevelContentHandle);
    ~Level();
    
    void OnInit();
    void OnLogicUpdate(TimeType);
    void OnRenderUpdate(TimeType);
    void OnQuit();

    Entity Instantiate(const Prefab&);

    Entity GetRootEntity() const;

private:
    Entity m_root_entity{};
    std::unordered_set<Entity> m_entities;

    void initRootEntity();
    void registerEntity(const EntityInstance&);

    void createEntityByPrefab(Entity entity, const Prefab& prefab);
};

class PlayerLogic: public EntityLogic {
public:
    using EntityLogic::EntityLogic;

    void OnInit() override;
    void OnLogicUpdate(TimeType) override;

private:
    enum class WalkDirection {
        Up,
        Left,
        Right,
        Down,
    };

    AnimationHandle m_walk_left;
    AnimationHandle m_walk_right;
    AnimationHandle m_walk_up;
    AnimationHandle m_walk_down;
    ImageHandle m_image_sheet;

    WalkDirection m_walk_direction = WalkDirection::Down;
};