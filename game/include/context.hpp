#pragma once
#include "entity.hpp"
#include "image.hpp"
#include "input/finger_touch.hpp"
#include "input/gamepad.hpp"
#include "input/input.hpp"
#include "input/keyboard.hpp"
#include "input/mouse.hpp"
#include "inspector.hpp"
#include "renderer.hpp"
#include "window.hpp"

#include <memory>

class RelationshipManager;
class TransformManager;
class SpriteManager;

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

    void Update();
    void HandleEvents(const SDL_Event&);
    bool ShouldExit();

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<ImageManager> m_image_manager;
    std::unique_ptr<Inspector> m_inspector;
    std::unique_ptr<TransformManager> m_transform_manager;
    std::unique_ptr<SpriteManager> m_sprite_manager;
    std::unique_ptr<RelationshipManager> m_relationship_manager;
    std::unique_ptr<Keyboard> m_keyboard;
    std::unique_ptr<Mouse> m_mouse;
    std::unique_ptr<Touches> m_touchs;
    std::unique_ptr<GamepadManager> m_gamepad_manager;
    std::unique_ptr<InputManager> m_input_manager;

    Entity GetRootEntity();

private:
    static std::unique_ptr<Context> instance;

    bool m_should_exit = false;
    Entity m_last_entity = 0;
    Entity m_root_entity;

    Context();

    void logicUpdate();
    void gameLogicUpdate();
    void logicPostUpdate();
    void renderUpdate();
    Entity createEntity();
};