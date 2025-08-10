#include "context.hpp"
#include "imgui.h"
#include "level.hpp"
#include "log.hpp"
#include "path.hpp"
#include "rapidxml_print.hpp"
#include "rapidxml_utils.hpp"
#include "relationship.hpp"
#include "schema/config.hpp"
#include "schema/serialize/asset_extensions.hpp"
#include "schema/serialize/config.hpp"
#include "schema/serialize/input.hpp"
#include "schema/serialize/prefab.hpp"
#include "sdl_call.hpp"
#include "serialize.hpp"
#include "sprite.hpp"
#include "storage.hpp"
#include "tilemap.hpp"
#include "transform.hpp"
#include "uuid.h"

#include <iostream>

std::unique_ptr<Context> Context::instance;

void Context::Init() {
    if (!instance) {
        instance = std::unique_ptr<Context>(new Context());
        instance->Initialize();
    } else {
        LOGW("inited context singleton twice!");
    }
}

void Context::Destroy() {
    instance->Shutdown();
    instance.reset();
}

Context& GAME_CONTEXT {
    return *instance;
}

void Context::Shutdown() {
    m_level_manager->Switch({});
    m_level_manager.reset();
}

Context::~Context() {
    m_debug_drawer.reset();
    m_event_system.reset();
    m_entity_logic_manager.reset();
    m_cct_manager.reset();
    m_physics_scene.reset();
    m_time.reset();
    m_input_manager.reset();
    m_assets_manager.reset();
    m_gamepad_manager.reset();

#ifdef TL_ENABLE_EDITOR
    m_editor.reset();
#endif

    m_touches.reset();
    m_mouse.reset();
    m_keyboard.reset();
    m_tilemap_component_manager.reset();
    m_sprite_manager.reset();
    m_transform_manager.reset();
    m_inspector.reset();
    m_animation_player_manager.reset();
    m_assets_manager.reset();
    m_renderer.reset();
    m_window.reset();
    SDL_Quit();
    LOGI("game exits");
}

void Context::Initialize() {
    auto handle = m_assets_manager->GetManager<GameConfig>().Load(
        std::string{"assets/gpa/game_config"} +
        GameConfig_AssetExtension.data());
    if (!handle) {
        LOGC("game config not found!");
        SDL_Quit();
        return;
    }

    m_game_config = *handle;
    m_assets_manager->GetManager<GameConfig>().Unload(handle);

    m_input_manager->Initialize(
        m_assets_manager->GetManager<InputConfig>().Load(
            m_game_config.m_input_config_asset));

    m_level_manager->Switch(m_assets_manager->GetManager<Level>().Load(
        m_game_config.m_basic_level_asset));
}

void Context::Update() {
    auto elapse_time = m_time->GetElapseTime();

    logicUpdate(elapse_time);
    renderUpdate(elapse_time);
    logicPostUpdate(elapse_time);

    m_level_manager->PoseUpdate();
}

void Context::HandleEvents(const SDL_Event& event) {
    m_inspector->HandleEvents(event);
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

bool Context::ShouldExit() const {
    return m_should_exit;
}

const GameConfig& Context::GetGameConfig() const {
    return m_game_config;
}

#ifdef TL_ENABLE_EDITOR
const Path& Context::GetProjectPath() const {
    return m_project_path;
}
#endif

Context::Context() {
    SDL_CALL(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
                      SDL_INIT_GAMEPAD));
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

#ifdef TL_ENABLE_EDITOR
    parseProjectPath();
#endif
    m_assets_manager = std::make_unique<AssetsManager>();

    m_window = std::make_unique<Window>("TreasureLooter", 1024, 720);
    m_renderer = std::make_unique<Renderer>(*m_window);
    m_renderer->SetClearColor({0.3, 0.3, 0.3, 1});

    m_tilemap_component_manager = std::make_unique<TilemapComponentManager>();
    m_animation_player_manager = std::make_unique<AnimationPlayerManager>();

    m_inspector = std::make_unique<Inspector>(*m_window, *m_renderer);

#ifdef TL_ENABLE_EDITOR
    m_editor = std::make_unique<Editor>();
#endif

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
    m_entity_logic_manager = std::make_unique<EntityLogicManager>();
    m_level_manager = std::make_unique<LevelManager>();

#ifdef TL_DEBUG
    m_debug_drawer = std::make_unique<DebugDrawer>();
#else
    m_debug_drawer = std::make_unique<TrivialDebugDrawer>();
#endif
}

void Context::logicUpdate(TimeType elapse) {
    m_time->Update();
    m_gamepad_manager->Update();
    m_keyboard->Update();
    m_mouse->Update();
    m_touches->Update();

    m_level_manager->UpdateLogic(elapse);

    m_animation_player_manager->Update(elapse);
    m_relationship_manager->Update();
    m_event_system->Update();
}

void Context::logicPostUpdate(TimeType elapse) {
    m_mouse->PostUpdate();
    m_touches->PostUpdate();
}

void Context::renderUpdate(TimeType elapse) {
    m_inspector->BeginFrame();
    m_renderer->Clear();

    m_tilemap_component_manager->Update();
    m_sprite_manager->Update();

    m_level_manager->UpdateRender(elapse);

    m_physics_scene->RenderDebug();
    m_cct_manager->RenderDebug();

    m_inspector->Update();

#ifdef TL_ENABLE_EDITOR
    m_editor->Update();
#endif

    m_debug_drawer->Update(m_time->GetElapseTime());

    m_inspector->EndFrame();
    m_renderer->Present();
}

Entity Context::CreateEntity() {
    return static_cast<Entity>(m_last_entity++);
}

void Context::parseProjectPath() {
    auto file =
        IOStream::CreateFromFile("project_path.xml", IOMode::Read, true);
    if (!file) {
        return;
    }

    auto content = file->Read();
    content.push_back('\0');

    rapidxml::xml_document<> doc;
    doc.parse<0>(content.data());
    auto node = doc.first_node("ProjectPath");
    if (!node) {
        LOGE("no ProjectPath node in project_path.xml");
        return;
    }

#ifdef TL_ENABLE_EDITOR
    m_project_path = node->value();
#endif
}