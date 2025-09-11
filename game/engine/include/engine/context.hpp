#pragma once
#include "animation_player.hpp"
#include "bind_point.hpp"
#include "camera.hpp"
#include "cct.hpp"
#include "debug_drawer.hpp"
#include "entity.hpp"
#include "event.hpp"
#include "input/finger_touch.hpp"
#include "input/gamepad.hpp"
#include "input/input.hpp"
#include "input/keyboard.hpp"
#include "input/mouse.hpp"
#include "level.hpp"
#include "physics.hpp"
#include "renderer.hpp"
#include "schema/config.hpp"
#include "tilemap.hpp"
#include "timer.hpp"
#include "trigger.hpp"
#include "window.hpp"

#include <memory>

#include "motor.hpp"

struct EntityInstance;
class RelationshipManager;
class TransformManager;
class SpriteManager;
class AssetsManager;
class Level;
struct GameConfig;

class CommonContext {
public:
    static void ChangeContext(CommonContext&);

    static CommonContext& GetInst();

    virtual ~CommonContext();

    /**
     * initialize 3rdlib systems
     */
    virtual void InitSystem();

    /**
     * shutdown 3rdlib systems
     */
    virtual void ShutdownSystem();

    /**
     * initialize context
     */
    virtual void Initialize();

    /**
     * shutdown context
     */
    virtual void Shutdown();

    virtual void Update() = 0;
    virtual void HandleEvents(const SDL_Event&);

    bool IsRunning() const;

    Entity CreateEntity();

    bool ShouldExit() const;
    void Exit();

    const GameConfig& GetGameConfig() const;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<EventSystem> m_event_system;
    std::unique_ptr<InputManager> m_input_manager;
    std::unique_ptr<Keyboard> m_keyboard;
    std::unique_ptr<Mouse> m_mouse;
    std::unique_ptr<Touches> m_touches;
    std::unique_ptr<GamepadManager> m_gamepad_manager;
    std::unique_ptr<AssetsManager> m_assets_manager;
    std::unique_ptr<TransformManager> m_transform_manager;
    std::unique_ptr<SpriteManager> m_sprite_manager;
    std::unique_ptr<RelationshipManager> m_relationship_manager;
    std::unique_ptr<Time> m_time;
    std::unique_ptr<PhysicsScene> m_physics_scene;
    std::unique_ptr<CCTManager> m_cct_manager;
    std::unique_ptr<LevelManager> m_level_manager;
    std::unique_ptr<IDebugDrawer> m_debug_drawer;
    std::unique_ptr<TimerManager> m_timer_manager;
    std::unique_ptr<MotorManager> m_motor_manager;
    std::unique_ptr<BindPointsComponentManager> m_bind_point_component_manager;
    std::unique_ptr<TriggerComponentManager> m_trigger_component_manager;
    std::unique_ptr<AnimationPlayerManager> m_animation_player_manager;
    std::unique_ptr<TilemapComponentManager> m_tilemap_component_manager;
    Camera m_camera;

protected:
    void beginImGui();
    void endImGui();

private:
    bool m_should_exit = false;
    std::underlying_type_t<Entity> m_last_entity = 1;

    void initImGui();
    void shutdownImGui();

    static CommonContext* m_current_context;
    GameConfig m_game_config;
};

class GameContext : public CommonContext {
public:
    static void Init();
    static void Destroy();
    static GameContext& GetInst();

    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;
    GameContext(GameContext&&) = delete;
    GameContext& operator=(GameContext&&) = delete;
    ~GameContext() override;

    void Initialize() override;

    void Update() override;

private:
    static std::unique_ptr<GameContext> instance;

    void logicUpdate(TimeType elapse);
    void logicPostUpdate(TimeType elapse);
    void renderUpdate(TimeType elapse);

    GameContext() = default;
};

#define GAME_CONTEXT ::GameContext::GetInst()
#define CURRENT_CONTEXT ::CommonContext::GetInst()