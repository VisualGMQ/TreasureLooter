#include "engine/context.hpp"
#include "SDL3_ttf/SDL_ttf.h"
#include "engine/asset_manager.hpp"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "imgui.h"
#include "engine/level.hpp"
#include "engine/log.hpp"
#include "engine/relationship.hpp"
#include "schema/config.hpp"
#include "schema/serialize/asset_extensions.hpp"
#include "schema/serialize/input.hpp"
#include "schema/serialize/prefab.hpp"
#include "engine/sdl_call.hpp"
#include "engine/serialize.hpp"
#include "engine/sprite.hpp"
#include "engine/storage.hpp"
#include "engine/tilemap.hpp"
#include "engine/transform.hpp"
#include "uuid.h"

std::unique_ptr<GameContext> GameContext::instance;

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
    m_bind_point_component_manager = std::make_unique<
        BindPointsComponentManager>();
    m_animation_player_manager = std::make_unique<AnimationPlayerManager>();
    m_tilemap_component_manager = std::make_unique<TilemapComponentManager>();
    m_debug_drawer = std::make_unique<DebugDrawer>();
}

void CommonContext::Shutdown() {
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
    m_assets_manager.reset();
    m_gamepad_manager.reset();

    m_touches.reset();
    m_mouse.reset();
    m_keyboard.reset();
    m_tilemap_component_manager.reset();
    m_sprite_manager.reset();
    m_transform_manager.reset();
    m_animation_player_manager.reset();
    m_assets_manager.reset();
    shutdownImGui();
    m_renderer.reset();
    m_window.reset();
}

void CommonContext::HandleEvents(const SDL_Event& event) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT) {
        m_should_exit = true;
    } else if (event.type == SDL_EVENT_KEY_UP ||
               event.type == SDL_EVENT_KEY_DOWN) {
        m_keyboard->HandleEvent(event.key);
    } else if (event.type == SDL_EVENT_FINGER_UP ||
               event.type == SDL_EVENT_FINGER_DOWN ||
               event.type == SDL_EVENT_FINGER_MOTION ||
               event.type == SDL_EVENT_FINGER_CANCELED) {
        m_touches->HandleEvent(event.tfinger);
    }
    m_gamepad_manager->HandleEvent(event);
    m_mouse->HandleEvent(event);

    m_event_system->HandleEvent(event);
}

Entity CommonContext::CreateEntity() {
    return static_cast<Entity>(m_last_entity++);
}

bool CommonContext::ShouldExit() const {
    return m_should_exit;
}

void CommonContext::Exit() {
    m_should_exit = true;
}

void CommonContext::beginImGui() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void CommonContext::endImGui() {
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
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(
        main_scale); // Bake a fixed style scale. (until we have a solution for
    // dynamic style scaling, changing this requires resetting
    // Style + calling this again)
    style.FontScaleDpi = main_scale; // Set initial font scale. (using
    // io.ConfigDpiScaleFonts=true makes this unnecessary. We
    // leave both here for documentation purpose)

    ImGui_ImplSDL3_InitForSDLRenderer(m_window->GetWindow(),
                                      m_renderer->GetRenderer());
    ImGui_ImplSDLRenderer3_Init(m_renderer->GetRenderer());
}

void CommonContext::shutdownImGui() {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
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

void GameContext::Initialize() {
#ifdef TL_DEBUG
    m_debug_drawer = std::make_unique<DebugDrawer>();
#else
    m_debug_drawer = std::make_unique<TrivialDebugDrawer>();
#endif

    CommonContext::Initialize();

    auto handle = m_assets_manager->GetManager<GameConfig>().Load(
        std::string{"assets/gpa/game_config"} +
        GameConfig_AssetExtension.data());
    if (!handle) {
        LOGC("game config not found!");
        SDL_Quit();
        return;
    }

    m_game_config = *handle;
    m_camera.ChangeScale(GetGameConfig().m_camera_scale);
    m_assets_manager->GetManager<GameConfig>().Unload(handle);
    m_input_manager->Initialize(
        m_assets_manager->GetManager<InputConfig>().Load(
            m_game_config.m_input_config_asset), *this);

    m_level_manager->Switch(m_assets_manager->GetManager<Level>().Load(
        m_game_config.m_basic_level_asset));
}

void GameContext::Update() {
    auto elapse_time = m_time->GetElapseTime();

    logicUpdate(elapse_time);
    renderUpdate(elapse_time);
    logicPostUpdate(elapse_time);

    m_level_manager->PoseUpdate();
}

const GameConfig& GameContext::GetGameConfig() const {
    return m_game_config;
}

void GameContext::logicUpdate(TimeType elapse) {
    m_time->Update();
    m_gamepad_manager->Update();
    m_keyboard->Update();
    m_mouse->Update();
    m_touches->Update();

    m_motor_manager->Update(elapse);
    m_bind_point_component_manager->Update();

    m_animation_player_manager->Update(elapse);
    m_relationship_manager->Update();
    m_trigger_component_manager->Update();
    m_event_system->Update();
    m_timer_manager->Update(elapse);
}

void GameContext::logicPostUpdate(TimeType elapse) {
    m_mouse->PostUpdate();
    m_touches->PostUpdate();
}

void GameContext::renderUpdate(TimeType elapse) {
    m_renderer->Clear();
    beginImGui();

    m_tilemap_component_manager->Update();
    m_sprite_manager->Update();

    m_physics_scene->RenderDebug();

    m_debug_drawer->Update(m_time->GetElapseTime());

    endImGui();
    m_renderer->Present();
}

GameContext::~GameContext() {}
