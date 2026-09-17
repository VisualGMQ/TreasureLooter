#pragma once
#include "client/camera.hpp"
#include "client/hfsm.hpp"
#include "common/context.hpp"
#include "common/net/udp.hpp"
#include "tilemap_layer_collision_component.hpp"

class ClientTilemapDetourManager;
class IDebugDrawer;
class GamepadManager;
class Touches;
class Mouse;
class Keyboard;
class SpriteManager;
class DrawOrderManager;
class PresentTransformManager;
class PlayerController;
class UIComponentManager;
class AnimationManager;
class AnimationPlayerManager;
class TilemapLayerRenderComponentManager;
class InputManager;
class Window;
class GameplayConfigManager;
class DebugPanel;

class ClientContext : public CommonContext {
public:
    static void ChangeContext(ClientContext&);

    static void Init();
    static void Destroy();
    static ClientContext& GetInst();

    ClientContext(const ClientContext&) = delete;
    ClientContext& operator=(const ClientContext&) = delete;
    ClientContext(ClientContext&&) = delete;
    ClientContext& operator=(ClientContext&&) = delete;
    ~ClientContext() override;

    void InitSystem() override;
    void Initialize(int argc, char** argv) override;
    void Shutdown() override;

    void HandleEvents(const SDL_Event&) override;
    void Update() override;

    void ConnectToServer(const NetAddress&);

    void AttachComponentsOnLogicEntity(LogicEntity,
                                       const EntityInstance&) override;
    void RemoveAllComponentsOnLogicEntity(LogicEntity) override;
    /**
     * remove the logic entity and its corresponding present entity.
     */
    void RemoveEntity(LogicEntity) override;

    PresentEntity CreatePresentEntity(LogicEntity logic_entity);
    [[nodiscard]] PresentEntity GetPresentEntity(LogicEntity logic_entity) const;

    void AttachComponentsOnPresentEntity(PresentEntity,
                                         const EntityInstance&);
    void RemoveAllComponentsOnPresentEntity(PresentEntity);
    /**
     * remove all render components of the present entity corresponding to the
     * given logic entity.
     */
    void RemovePresentEntity(LogicEntity logic_entity);

    [[nodiscard]] const ClientConfig& GetConfig() const;

    [[nodiscard]] Vec2 WindowCoordToWorld(const Vec2& window_pos) const;
    [[nodiscard]] Vec2 WorldCoordToWindow(const Vec2& world_pos) const;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<PlayerController> m_player_controller;
    std::unique_ptr<Keyboard> m_keyboard;
    std::unique_ptr<Mouse> m_mouse;
    std::unique_ptr<Touches> m_touches;
    std::unique_ptr<GamepadManager> m_gamepad_manager;
    std::unique_ptr<SpriteManager> m_sprite_manager;
    std::unique_ptr<PresentTransformManager> m_present_transform_manager;
    std::unique_ptr<AnimationPlayerManager> m_animation_player_manager;
    std::unique_ptr<DrawOrderManager> m_draw_order_manager;
    std::unique_ptr<InputManager> m_input_manager;
    std::unique_ptr<UIComponentManager> m_ui_manager;
    std::unique_ptr<TilemapLayerRenderComponentManager>
        m_tilemap_layer_render_component_manager;
    std::unique_ptr<Renderer> m_renderer;
    UDPPeer m_net_peer;
    Camera m_camera;
    std::unique_ptr<DebugPanel> m_debug_panel;
    std::unique_ptr<ClientHFSMDebugger> m_hfsm_debugger;

protected:
    void beginImGui();
    void endImGui();

    ClientContext() = default;

    static std::unique_ptr<ClientContext> instance;

    ClientTilemapDetourManager* m_client_tilemap_detour_manager{};
    ClientTilemapLayerCollisionComponentManager*
        m_client_tilemap_layer_collision_component_manager{};

private:
    void initImGui();
    void shutdownImGui();

    void logicUpdate(TimeType elapse);
    void logicPostUpdate(TimeType elapse);
    void renderUpdate(TimeType elapse);

    void syncPresentTransform(LogicEntity entity, class Transform* parent);
    void removePresentEntityWithChildren(LogicEntity entity);

    void initClientConfig();
    void registerAllDebugCommands();

    ClientConfig m_config;
};

#define CLIENT_CONTEXT ::ClientContext::GetInst()
