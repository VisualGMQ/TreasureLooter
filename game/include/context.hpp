#pragma once
#include "animation_player.hpp"
#include "cct.hpp"
#include "debug_drawer.hpp"
#include "editor/editor.hpp"
#include "entity.hpp"
#include "entity_logic.hpp"
#include "event.hpp"
#include "input/finger_touch.hpp"
#include "input/gamepad.hpp"
#include "input/input.hpp"
#include "input/keyboard.hpp"
#include "input/mouse.hpp"
#include "inspector.hpp"
#include "level.hpp"
#include "physics.hpp"
#include "renderer.hpp"
#include "schema/config.hpp"
#include "tilemap.hpp"
#include "timer.hpp"
#include "window.hpp"

#include <memory>

struct EntityInstance;
class RelationshipManager;
class TransformManager;
class SpriteManager;
class AssetsManager;
class Level;

class Context {
public:
    static void Init();
    static void Destroy();
    static Context& GetInst();

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(Context&&) = delete;
    ~Context();

    void Initialize();
    void Shutdown();

    void Update();
    void HandleEvents(const SDL_Event&);
    bool ShouldExit();

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<AnimationPlayerManager> m_animation_player_manager;
    std::unique_ptr<AssetsManager> m_assets_manager;
    std::unique_ptr<Inspector> m_inspector;
#ifdef TL_ENABLE_EDITOR
    std::unique_ptr<Editor> m_editor;
#endif
    std::unique_ptr<TransformManager> m_transform_manager;
    std::unique_ptr<TilemapComponentManager> m_tilemap_component_manager;
    std::unique_ptr<SpriteManager> m_sprite_manager;
    std::unique_ptr<RelationshipManager> m_relationship_manager;
    std::unique_ptr<Keyboard> m_keyboard;
    std::unique_ptr<Mouse> m_mouse;
    std::unique_ptr<Touches> m_touches;
    std::unique_ptr<GamepadManager> m_gamepad_manager;
    std::unique_ptr<InputManager> m_input_manager;
    std::unique_ptr<Time> m_time;
    std::unique_ptr<PhysicsScene> m_physics_scene;
    std::unique_ptr<CCTManager> m_cct_manager;
    std::unique_ptr<EventSystem> m_event_system;
    std::unique_ptr<EntityLogicManager> m_entity_logic_manager;
    std::unique_ptr<LevelManager> m_level_manager;
    std::unique_ptr<IDebugDrawer> m_debug_drawer;

    Entity CreateEntity();

#ifdef TL_ENABLE_EDITOR
    const Path& GetProjectPath() const;
#endif

private:
    static std::unique_ptr<Context> instance;

    bool m_should_exit = false;
    std::underlying_type_t<Entity> m_last_entity = 1;
    GameConfigHandle m_game_config;

#ifdef TL_ENABLE_EDITOR
    Path m_project_path;  // only used for editor
#endif

    Context();

    void logicUpdate(TimeType elapse);
    void logicPostUpdate(TimeType elapse);
    void renderUpdate(TimeType elapse);

    void parseProjectPath();
};

#define GAME_CONTEXT Context::GetInst()