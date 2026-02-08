#pragma once
#include "engine/entity.hpp"
#include "schema/config.hpp"
#include "engine/camera.hpp"
#include <memory>

class TriggerComponentManager;
class BindPointsComponentManager;
class IDebugDrawer;
class CCTManager;
class GamepadManager;
class Touches;
class Mouse;
class Keyboard;
struct EntityInstance;
class RelationshipManager;
class TransformManager;
class SpriteManager;
class AssetsManager;
class Level;
struct GameConfig;
class PlayerController;
class UIComponentManager;
class ScriptComponentManager;
class AnimationManager;
class AnimationPlayerManager;
class InputManager;
class Window;
class Renderer;
class PhysicsScene;
class GameplayConfigManager;
class TilemapComponent;
class TilemapComponentManager;
class LevelManager;

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
    bool IsInited() const;
    void Exit();

    const GameConfig& GetGameConfig() const;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<EventSystem> m_event_system;
    std::unique_ptr<InputManager> m_input_manager;
    std::unique_ptr<PlayerController> m_player_controller;
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
    std::unique_ptr<UIComponentManager> m_ui_manager;
    std::unique_ptr<BindPointsComponentManager> m_bind_point_component_manager;
    std::unique_ptr<TriggerComponentManager> m_trigger_component_manager;
    std::unique_ptr<AnimationPlayerManager> m_animation_player_manager;
    std::unique_ptr<TilemapComponentManager> m_tilemap_component_manager;
    std::unique_ptr<ScriptComponentManager> m_script_component_manager;
    std::unique_ptr<GameplayConfigManager> m_gameplay_config_manager;
    Camera m_camera;

protected:
    void beginImGui();
    void endImGui();
    class ImGuiContext* m_imgui_context{};

private:
    static CommonContext* m_current_context;

    bool m_should_exit = true;
    bool m_is_inited = false;
    std::underlying_type_t<Entity> m_last_entity = 1;
    GameConfig m_game_config;

    void initImGui();
    void shutdownImGui();
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
