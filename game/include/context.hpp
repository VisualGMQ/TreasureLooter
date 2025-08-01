#pragma once
#include "animation.hpp"
#include "animation_player.hpp"
#include "editor/editor.hpp"
#include "entity.hpp"
#include "input/finger_touch.hpp"
#include "input/gamepad.hpp"
#include "input/input.hpp"
#include "input/keyboard.hpp"
#include "input/mouse.hpp"
#include "inspector.hpp"
#include "renderer.hpp"
#include "tilemap.hpp"
#include "timer.hpp"
#include "window.hpp"

#include <memory>

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

    std::unique_ptr<Level> m_level;

    Entity CreateEntity();

#ifdef TL_ENABLE_EDITOR
    const Path& GetProjectPath() const;
#endif

    void RegisterEntity(const EntityInstance&);
    void RemoveEntity(Entity);

private:
    static std::unique_ptr<Context> instance;

    bool m_should_exit = false;
    Entity m_last_entity = 0;

#ifdef TL_ENABLE_EDITOR
    Path m_project_path; // only used for editor
#endif

    Context();

    void logicUpdate();
    void logicPostUpdate();
    void renderUpdate();

    void parseProjectPath();
};

#define GAME_CONTEXT Context::GetInst()