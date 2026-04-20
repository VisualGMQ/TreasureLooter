#pragma once
#include "client/camera.hpp"
#include "common/context.hpp"

class IDebugDrawer;
class GamepadManager;
class Touches;
class Mouse;
class Keyboard;
class SpriteManager;
class DrawOrderManager;
class PlayerController;
class UIComponentManager;
class AnimationManager;
class AnimationPlayerManager;
class TilemapLayerRenderComponentManager;
class InputManager;
class Window;
class GameplayConfigManager;

class ClientContext : public CommonContext {
public:
    static void Init();
    static void Destroy();
    static ClientContext& GetInst();

    ClientContext(const ClientContext&) = delete;
    ClientContext& operator=(const ClientContext&) = delete;
    ClientContext(ClientContext&&) = delete;
    ClientContext& operator=(ClientContext&&) = delete;
    ~ClientContext() override;

    void Initialize(int argc, char** argv) override;
    void Shutdown() override;

    void HandleEvents(const SDL_Event&) override;
    void Update() override;

    void AttachComponentsOnEntity(Entity, const EntityInstance&) override;
    void RemoveAllComponentsOnEntity(Entity) override;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<PlayerController> m_player_controller;
    std::unique_ptr<Keyboard> m_keyboard;
    std::unique_ptr<Mouse> m_mouse;
    std::unique_ptr<Touches> m_touches;
    std::unique_ptr<GamepadManager> m_gamepad_manager;
    std::unique_ptr<SpriteManager> m_sprite_manager;
    std::unique_ptr<AnimationPlayerManager> m_animation_player_manager;
    std::unique_ptr<DrawOrderManager> m_draw_order_manager;
    std::unique_ptr<InputManager> m_input_manager;
    std::unique_ptr<UIComponentManager> m_ui_manager;
    std::unique_ptr<TilemapLayerRenderComponentManager>
        m_tilemap_layer_render_component_manager;
    std::unique_ptr<Renderer> m_renderer;
    Camera m_camera;

protected:
    void beginImGui();
    void endImGui();

    ClientContext() = default;

private:
    static std::unique_ptr<ClientContext> instance;

    void initImGui();
    void shutdownImGui();

    void logicUpdate(TimeType elapse);
    void logicPostUpdate(TimeType elapse);
    void renderUpdate(TimeType elapse);
};

#define CLIENT_CONTEXT ::ClientContext::GetInst()
