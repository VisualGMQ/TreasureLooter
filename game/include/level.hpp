#pragma once
#include "animation.hpp"
#include "entity_logic.hpp"
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

    void OnEnter();
    void OnInit();
    void OnLogicUpdate(TimeType);
    void OnRenderUpdate(TimeType);
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
    std::unordered_map<PrefabHandle, std::vector<Entity>> m_prefab_entity_map;
#endif

    void initRootEntity();
    void registerEntity(const EntityInstance&);

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

    void UpdateLogic(TimeType t);
    void UpdateRender(TimeType t);
    void PoseUpdate();

    LevelHandle GetCurrentLevel() const;

private:
    LevelHandle m_level;
};

class PlayerLogic : public EntityLogic {
public:
    using EntityLogic::EntityLogic;

    void OnInit() override;
    void OnLogicUpdate(TimeType) override;
    void OnQuit() override;

private:
    enum class WalkDirection {
        Up,
        Left,
        Right,
        Down,
    };

    struct TouchJoystick {
        Circle m_circle;
    };

    TimerID m_timer = null_timer_id;

    AnimationHandle m_walk_left;
    AnimationHandle m_walk_right;
    AnimationHandle m_walk_up;
    AnimationHandle m_walk_down;
    ImageHandle m_image_sheet;
    EventListenerID m_gamepad_event_listener;
    EventListenerID m_window_resize_event_listener;
    SDL_JoystickID m_gamepad_id = 0;

    TouchJoystick m_touch_joystick;
    Circle m_finger_attack_button;

    std::optional<size_t> m_move_finger_idx;
    std::optional<size_t> m_attack_finger_idx;

    WalkDirection m_walk_direction = WalkDirection::Down;

    void handleFingerTouchJoystick();
};