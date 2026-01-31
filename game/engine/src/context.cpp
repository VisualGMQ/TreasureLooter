#include "engine/context.hpp"
#include "SDL3_ttf/SDL_ttf.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "engine/asset_manager.hpp"
#include "engine/level.hpp"
#include "engine/log.hpp"
#include "engine/relationship.hpp"
#include "engine/script/script.hpp"
#include "engine/sdl_call.hpp"
#include "engine/serialize.hpp"
#include "engine/sprite.hpp"
#include "engine/storage.hpp"
#include "engine/tilemap.hpp"
#include "engine/transform.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "schema/asset_info.hpp"
#include "schema/config.hpp"
#include "schema/serialize/input.hpp"
#include "schema/serialize/prefab.hpp"
#include "engine/ui.hpp"
#include "engine/uuid.hpp"
#include "engine/profile.hpp"
#include <memory>
#include <csignal>

std::unique_ptr<GameContext> GameContext::instance;

CommonContext* CommonContext::m_current_context{};

void CommonContext::ChangeContext(CommonContext& ctx) {
    m_current_context = &ctx;
}

CommonContext& CommonContext::GetInst() {
    return *m_current_context;
}

CommonContext::~CommonContext() {}

void CommonContext::InitSystem() {
    LOGT("system init");
    SDL_CALL(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
                      SDL_INIT_GAMEPAD));
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

    SDL_CALL(TTF_Init());
}

void CommonContext::ShutdownSystem() {
    TTF_Quit();
    SDL_Quit();
    LOGT("system shutdown");
}

void CommonContext::Initialize() {
    m_should_exit = false;
    m_is_inited = true;
    m_assets_manager = std::make_unique<AssetsManager>();

    m_window = std::make_unique<Window>("TreasureLooter", 1024, 720);
    m_renderer = std::make_unique<Renderer>(*m_window);
    m_renderer->SetClearColor({0.3, 0.3, 0.3, 1});
    initImGui();

    m_transform_manager = std::make_unique<TransformManager>();
    m_sprite_manager = std::make_unique<SpriteManager>();
    m_relationship_manager = std::make_unique<RelationshipManager>();
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_touches = std::make_unique<Touches>();
    m_gamepad_manager = std::make_unique<GamepadManager>();

    m_input_manager = std::make_unique<InputManager>();

    m_time = std::make_unique<Time>();
    m_physics_scene = std::make_unique<PhysicsScene>();
    m_cct_manager = std::make_unique<CCTManager>();
    m_event_system = std::make_unique<EventSystem>();
    m_level_manager = std::make_unique<LevelManager>();
    m_trigger_component_manager = std::make_unique<TriggerComponentManager>();
    m_timer_manager = std::make_unique<TimerManager>();
    m_motor_manager = std::make_unique<MotorManager>();
    m_bind_point_component_manager =
        std::make_unique<BindPointsComponentManager>();
    m_animation_player_manager = std::make_unique<AnimationPlayerManager>();
    m_tilemap_component_manager = std::make_unique<TilemapComponentManager>();
    m_ui_manager = std::make_unique<UIComponentManager>();
    m_script_component_manager = std::make_unique<ScriptComponentManager>();

#ifdef TL_DEBUG
    m_debug_drawer = std::make_unique<DebugDrawer>();
#else
    m_debug_drawer = std::make_unique<TrivialDebugDrawer>();
#endif

    auto handle = m_assets_manager->GetManager<GameConfig>().Load(
        std::string{"assets/gpa/game_config"} +
        GameConfig_AssetExtension.data());
    if (!handle) {
        LOGC("game config not found!");
        SDL_Quit();
        return;
    }

    m_window->Resize(handle->m_logic_size);
    m_game_config = *handle;
    m_camera.ChangeScale(GetGameConfig().m_camera_scale);
    m_assets_manager->GetManager<GameConfig>().Unload(handle);
}

void CommonContext::Shutdown() {
    m_should_exit = true;
    m_is_inited = false;
    if (m_level_manager) {
        m_level_manager->Switch({});
    }
    m_level_manager.reset();

    m_trigger_component_manager.reset();
    m_bind_point_component_manager.reset();
    m_timer_manager.reset();
    m_motor_manager.reset();
    m_debug_drawer.reset();
    m_event_system.reset();
    m_cct_manager.reset();
    m_physics_scene.reset();
    m_time.reset();
    m_input_manager.reset();
    m_gamepad_manager.reset();

    m_touches.reset();
    m_mouse.reset();
    m_keyboard.reset();
    m_tilemap_component_manager.reset();
    m_sprite_manager.reset();
    m_transform_manager.reset();
    m_animation_player_manager.reset();
    m_script_component_manager.reset();
    m_assets_manager.reset();
    shutdownImGui();
    m_renderer.reset();
    m_window.reset();
}

void CommonContext::HandleEvents(const SDL_Event& event) {
    ImGui::SetCurrentContext(m_imgui_context);
    ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        if (event.window.windowID == m_window->GetID()) {
            m_should_exit = true;
        }
    } else if (event.type == SDL_EVENT_KEY_UP ||
               event.type == SDL_EVENT_KEY_DOWN) {
        m_keyboard->HandleEvent(event.key);
    } else if (event.type == SDL_EVENT_FINGER_UP ||
               event.type == SDL_EVENT_FINGER_DOWN ||
               event.type == SDL_EVENT_FINGER_MOTION ||
               event.type == SDL_EVENT_FINGER_CANCELED) {
        m_touches->HandleEvent(event.tfinger);
    } else if (event.type == SDL_EVENT_WINDOW_RESIZED ||
               event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED ||
               event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED) {
        if (m_level_manager && event.window.windowID == m_window->GetID()) {
            m_event_system->EnqueueEvent(event.window);
        }
    }
    m_gamepad_manager->HandleEvent(event);
    m_mouse->HandleEvent(event);

    m_event_system->HandleEvent(event);
}

bool CommonContext::IsRunning() const {
    return !m_should_exit;
}

Entity CommonContext::CreateEntity() {
    return static_cast<Entity>(m_last_entity++);
}

bool CommonContext::ShouldExit() const {
    return m_should_exit;
}

bool CommonContext::IsInited() const {
    return m_is_inited;
}

void CommonContext::Exit() {
    m_should_exit = true;
}

const GameConfig& CommonContext::GetGameConfig() const {
    return m_game_config;
}

void CommonContext::beginImGui() {
    ImGui::SetCurrentContext(m_imgui_context);
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void CommonContext::endImGui() {
    ImGui::SetCurrentContext(m_imgui_context);
    ImGui::Render();
    auto& io = ImGui::GetIO();
    SDL_SetRenderScale(m_renderer->GetRenderer(), io.DisplayFramebufferScale.x,
                       io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                          m_renderer->GetRenderer());
}

void CommonContext::initImGui() {
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

void CommonContext::shutdownImGui() {
    if (m_imgui_context) {
        ImGui::SetCurrentContext(m_imgui_context);
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        m_imgui_context = nullptr;
    }
}

void GameContext::Init() {
    if (!instance) {
        instance = std::unique_ptr<GameContext>(new GameContext());
    } else {
        LOGW("inited context singleton twice!");
    }
}

void GameContext::Destroy() {
    instance.reset();
}

GameContext& GameContext::GetInst() {
    return *instance;
}

void sigintHandler(int signum) {
    if (signum == SIGINT) {
        GAME_CONTEXT.Exit();
    }
}

void GameContext::Initialize() {
    PROFILE_SECTION();

    signal(SIGINT, sigintHandler);
    
    CommonContext::Initialize();

    m_input_manager->Initialize(
        m_assets_manager->GetManager<InputConfig>().Load(
            GetGameConfig().m_input_config_asset),
        *this);

    m_level_manager->Switch(m_assets_manager->GetManager<Level>().Load(
        GetGameConfig().m_basic_level_asset));
}

void GameContext::Update() {
    PROFILE_MAIN_FRAME();

    {
        static bool first_execute = false;

        if (!first_execute) {
            ScriptBinaryDataHandle handle =
                m_assets_manager->GetManager<ScriptBinaryData>().Load(
                    "script/test.as");
            m_script_component_manager->RegisterEntity(
                m_level_manager->GetCurrentLevel()->GetRootEntity(), handle);
            first_execute = true;
        }
    }
    
    auto elapse_time = m_time->GetElapseTime();

    logicUpdate(elapse_time);
    renderUpdate(elapse_time);
    logicPostUpdate(elapse_time);

    m_level_manager->PoseUpdate();
}

void GameContext::logicUpdate(TimeType elapse) {
    PROFILE_SECTION();

    m_time->Update();
    m_gamepad_manager->Update();
    m_keyboard->Update();
    m_mouse->Update();
    m_touches->Update();

    m_script_component_manager->Update();
    m_motor_manager->Update(elapse);
    m_bind_point_component_manager->Update();

    m_animation_player_manager->Update(elapse);
    m_ui_manager->HandleEvent();
    m_ui_manager->Update();
    m_relationship_manager->Update();
    m_trigger_component_manager->Update();
    m_event_system->Update();
    m_timer_manager->Update(elapse);
}

void GameContext::logicPostUpdate(TimeType elapse) {
    PROFILE_SECTION();
    
    m_mouse->PostUpdate();
    m_touches->PostUpdate();
}

void GameContext::renderUpdate(TimeType elapse) {
    PROFILE_RENDERING_SECTION("renderUpdate");
    
    m_renderer->Clear();
    beginImGui();

    m_tilemap_component_manager->Update();
    m_sprite_manager->Update();
    m_ui_manager->Render();

    m_physics_scene->RenderDebug();

    m_debug_drawer->Update(m_time->GetElapseTime());

    endImGui();
    m_renderer->Present();
}

GameContext::~GameContext() {
    PROFILE_SHUTDOWN();
}
