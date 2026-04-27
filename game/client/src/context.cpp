#include "client/context.hpp"

#include "client/script/script_client_binding.hpp"
#include "common/script/script_binding.hpp"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "client/animation_player.hpp"
#include "client/asset_manager.hpp"
#include "client/controller.hpp"
#include "client/debug_drawer.hpp"
#include "client/draw.hpp"
#include "client/draw_order.hpp"
#include "client/input/finger_touch.hpp"
#include "client/input/gamepad.hpp"
#include "client/input/keyboard.hpp"
#include "client/input/mouse.hpp"
#include "client/renderer.hpp"
#include "client/scene.hpp"
#include "client/sprite.hpp"
#include "client/tilemap_render_component.hpp"
#include "client/ui.hpp"
#include "client/window.hpp"
#include "common/asset_manager.hpp"
#include "common/bind_point.hpp"
#include "common/cct.hpp"
#include "common/context.hpp"
#include "common/event.hpp"
#include "common/log.hpp"
#include "common/profile.hpp"
#include "common/relationship.hpp"
#include "common/scene.hpp"
#include "common/script/script.hpp"
#include "common/sdl_call.hpp"
#include "common/serialize.hpp"
#include "common/static_collision.hpp"
#include "common/storage.hpp"
#include "common/tilemap.hpp"
#include "common/tilemap_layer_collision_component.hpp"
#include "common/transform.hpp"
#include "common/trigger.hpp"
#include "common/uuid.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "schema/asset_info.hpp"
#include "schema/config.hpp"
#include "schema/serialize/input.hpp"
#include "schema/serialize/prefab.hpp"
#include <memory>

std::unique_ptr<ClientContext> ClientContext::instance;

void ClientContext::Init() {
    if (!instance) {
        instance = std::unique_ptr<ClientContext>(new ClientContext());
    } else {
        LOGW("inited context singleton twice!");
    }
}

void ClientContext::Destroy() {
    instance.reset();
}

ClientContext& ClientContext::GetInst() {
    return *instance;
}

void ClientContext::Initialize(int argc, char** argv) {
    PROFILE_SECTION();

    CommonContext::Initialize(argc, argv);
    m_assets_manager =
        std::unique_ptr<ClientAssetsManager>(new ClientAssetsManager{});
    m_scene_manager =
        std::unique_ptr<ClientSceneManager>(new ClientSceneManager{});
    m_script_binary_data_manager = std::make_unique<ScriptBinaryDataManager>();

    CommonContext::initGameConfig();

    auto& game_config = GetGameConfig();

    m_script_binary_data_manager->Initialize(game_config);
    m_script_binary_data_manager->BindModule([](lua_State* L) {
        BindTLModule(L);
        BindClientModule(L);
    });

    m_window = std::make_unique<Window>("TreasureLooter", 1024, 720);
    m_renderer = std::make_unique<Renderer>(*m_window);
    m_renderer->SetClearColor({0.3, 0.3, 0.3, 1});
    initImGui();

    m_ui_manager = std::make_unique<UIComponentManager>();
    m_sprite_manager = std::make_unique<SpriteManager>();
    m_draw_order_manager = std::make_unique<DrawOrderManager>();
    m_tilemap_layer_render_component_manager =
        std::make_unique<TilemapLayerRenderComponentManager>();

    // device input relate
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_touches = std::make_unique<Touches>();
    m_gamepad_manager = std::make_unique<GamepadManager>();

    // input relate
    m_input_manager = std::make_unique<InputManager>();
    m_player_controller = std::make_unique<PlayerController>(
        *m_input_manager, *m_event_system, *m_assets_manager,
        *m_transform_manager, *m_relationship_manager);

    m_animation_player_manager = std::make_unique<AnimationPlayerManager>();

#ifdef TL_DEBUG
    m_debug_drawer = std::unique_ptr<IDebugDrawer>(new DebugDrawer{});
#else
    m_debug_drawer = std::unique_ptr<IDebugDrawer>(new TrivialDebugDrawer{});
#endif

    m_window->Resize(game_config.m_logic_size);
    m_camera.ChangeScale(game_config.m_camera_scale);

    m_input_manager->Initialize(
        m_assets_manager->GetManager<InputConfig>().Load(
            game_config.m_input_config),
        *this);

    SceneHandle level =
        m_assets_manager->GetManager<Scene>().Load(game_config.m_entry_scene);
    m_scene_manager->Switch(level);

    m_player_controller->RegisterVirtualController(level, game_config);

    m_time->SetFPS(240);
}

void ClientContext::HandleEvents(const SDL_Event& event) {
    ImGui::SetCurrentContext(m_imgui_context);
    ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        if (event.window.windowID == m_window->GetID()) {
            Exit();
        }
    }
    if (event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_KEY_DOWN) {
        m_keyboard->HandleEvent(event.key);
    } else if (event.type == SDL_EVENT_FINGER_UP ||
               event.type == SDL_EVENT_FINGER_DOWN ||
               event.type == SDL_EVENT_FINGER_MOTION ||
               event.type == SDL_EVENT_FINGER_CANCELED) {
        m_touches->HandleEvent(event.tfinger);
    } else if (event.type == SDL_EVENT_WINDOW_RESIZED ||
               event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED ||
               event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED) {
        if (m_scene_manager && event.window.windowID == m_window->GetID()) {
            m_event_system->EnqueueEvent(event.window);
        }
    }
    m_gamepad_manager->HandleEvent(event);
    m_mouse->HandleEvent(event);
}

void ClientContext::Update() {
    PROFILE_FRAME_NAMED("main_loop");

    m_time->Begin();

    auto elapse_time = m_time->GetElapseTime();

    logicUpdate(elapse_time);
    renderUpdate(elapse_time);
    logicPostUpdate(elapse_time);

    m_scene_manager->PoseUpdate();

    m_time->End();
}

void ClientContext::AttachComponentsOnEntity(Entity entity,
                                             const EntityInstance& instance) {
    CommonContext::AttachComponentsOnEntity(entity, instance);

    auto& prefab = *instance.m_prefab;

    if (prefab.m_sprite) {
        m_sprite_manager->RegisterEntity(entity, prefab.m_sprite.value());
    }
    if (prefab.m_tilemap_layer) {
        m_tilemap_layer_render_component_manager->RegisterEntity(
            entity, TilemapLayerRenderComponent{
                        entity, prefab.m_tilemap_layer.value()});
    }
    if (prefab.m_draw_order) {
        m_draw_order_manager->RegisterEntity(entity,
                                             prefab.m_draw_order.value());
    }
    if (prefab.m_animation) {
        m_animation_player_manager->RegisterEntity(entity,
                                                   prefab.m_animation.value());
    }
    if (prefab.m_ui) {
        m_ui_manager->RegisterEntity(entity, prefab.m_ui);
    }
}

void ClientContext::RemoveAllComponentsOnEntity(Entity entity) {
    m_sprite_manager->RemoveEntity(entity);
    m_ui_manager->RemoveEntity(entity);
    m_tilemap_layer_render_component_manager->RemoveEntity(entity);
    m_animation_player_manager->RemoveEntity(entity);
    m_draw_order_manager->RemoveEntity(entity);

    CommonContext::RemoveAllComponentsOnEntity(entity);
}

void ClientContext::logicUpdate(TimeType elapse) {
    PROFILE_SECTION();

    m_time->Update();
    m_gamepad_manager->Update();
    m_keyboard->Update();
    m_mouse->Update();
    m_touches->Update();

    m_script_component_manager->Update();

    m_animation_player_manager->Update(elapse);
    m_ui_manager->HandleEvent();
    m_ui_manager->Update();
    m_relationship_manager->Update();
    m_bind_point_component_manager->Update();
    m_static_collision_manager->Update();
    m_trigger_component_manager->Update();
    m_event_system->Update();
    m_timer_manager->Update(elapse);
}

void ClientContext::logicPostUpdate(TimeType elapse) {
    PROFILE_SECTION();

    m_mouse->PostUpdate();
    m_touches->PostUpdate();
}

void ClientContext::renderUpdate(TimeType elapse) {
    PROFILE_SECTION();

    m_renderer->Clear();
    beginImGui();

    m_draw_order_manager->Update();
    m_script_component_manager->Render();

    DrawCommandSubmitter draw_cmd_submitter;
    draw_cmd_submitter.Submit();
    m_renderer->ApplyDrawcall();

    draw_cmd_submitter.SubmitUI();
    m_renderer->ApplyDrawcall();

    m_physics_scene->RenderDebug();
    m_bind_point_component_manager->RenderDebug(elapse);
    m_debug_drawer->Update(m_time->GetElapseTime());
    m_renderer->ApplyDrawcall();

    endImGui();
    m_renderer->Present();
}

void ClientContext::Shutdown() {
    m_player_controller.reset();

    CommonContext::Shutdown();

    m_tilemap_layer_render_component_manager.reset();
    m_ui_manager.reset();
    m_draw_order_manager.reset();
    m_debug_drawer.reset();
    m_input_manager.reset();
    m_gamepad_manager.reset();

    m_touches.reset();
    m_mouse.reset();
    m_keyboard.reset();

    m_sprite_manager.reset();
    m_animation_player_manager.reset();
    shutdownImGui();
    m_renderer.reset();
    m_window.reset();
}

ClientContext::~ClientContext() {}

void ClientContext::beginImGui() {
    PROFILE_SECTION();

    ImGui::SetCurrentContext(m_imgui_context);
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void ClientContext::endImGui() {
    PROFILE_SECTION();

    ImGui::SetCurrentContext(m_imgui_context);
    ImGui::Render();
    auto& io = ImGui::GetIO();
    SDL_SetRenderScale(m_renderer->GetRenderer(), io.DisplayFramebufferScale.x,
                       io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                          m_renderer->GetRenderer());
}

void ClientContext::initImGui() {
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    IMGUI_CHECKVERSION();
    m_imgui_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_imgui_context);
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(
        main_scale);  // Bake a fixed style scale. (until we have a solution for
    // dynamic style scaling, changing this requires resetting
    // Style + calling this again)
    style.FontScaleDpi = main_scale;  // Set initial font scale. (using
    // io.ConfigDpiScaleFonts=true makes this unnecessary. We
    // leave both here for documentation purpose)

    ImGui_ImplSDL3_InitForSDLRenderer(m_window->GetWindow(),
                                      m_renderer->GetRenderer());
    ImGui_ImplSDLRenderer3_Init(m_renderer->GetRenderer());
}

void ClientContext::shutdownImGui() {
    if (m_imgui_context) {
        ImGui::SetCurrentContext(m_imgui_context);
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        m_imgui_context = nullptr;
    }
}
