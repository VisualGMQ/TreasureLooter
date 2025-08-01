#include "context.hpp"
#include "imgui.h"
#include "level.hpp"
#include "log.hpp"
#include "rapidxml_print.hpp"
#include "rapidxml_utils.hpp"
#include "relationship.hpp"
#include "schema/prefab.hpp"
#include "schema/serialize/asset_extensions.hpp"
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
    instance.reset();
}

Context& GAME_CONTEXT {
    return *instance;
}

Context::~Context() {
    if (m_level) {
        m_level->OnQuit();
    }
    m_level.reset();

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
    m_level = std::make_unique<GameLevel>();
    m_level->OnInit();
}

void Context::Update() {
    logicUpdate();
    renderUpdate();
    logicPostUpdate();
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
}

bool Context::ShouldExit() {
    return m_should_exit;
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

    m_window = std::make_unique<Window>("TreasureLooter", 1024, 720);
    m_renderer = std::make_unique<Renderer>(*m_window);
    m_renderer->SetClearColor({0.3, 0.3, 0.3, 1});

    m_tilemap_component_manager = std::make_unique<TilemapComponentManager>();
    m_animation_player_manager = std::make_unique<AnimationPlayerManager>();
    m_assets_manager = std::make_unique<AssetsManager>();

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

    m_input_manager = std::make_unique<InputManager>(
        *this, std::string{"assets/gpa/input_config"} +
                   InputConfig_AssetExtension.data());

    m_time = std::make_unique<Time>();
}

void Context::logicUpdate() {
    m_time->Update();
    m_gamepad_manager->Update();
    m_keyboard->Update();
    m_mouse->Update();
    m_touches->Update();

    if (m_level) {
        m_level->OnUpdate(m_time->GetElapseTime());
    }

    m_animation_player_manager->Update(m_time->GetElapseTime());
    m_relationship_manager->Update();
}

void Context::logicPostUpdate() {
    m_mouse->PostUpdate();
    m_touches->PostUpdate();
}

void Context::renderUpdate() {
    m_inspector->BeginFrame();
    m_renderer->Clear();

    m_tilemap_component_manager->Update();
    m_sprite_manager->Update();
    m_inspector->Update();

#ifdef TL_ENABLE_EDITOR
    m_editor->Update();
#endif

    m_inspector->EndFrame();
    m_renderer->Present();
}

Entity Context::CreateEntity() {
    return m_last_entity++;
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

void Context::RegisterEntity(const EntityInstance& entity_instance) {
    if (entity_instance.m_data.m_sprite) {
        m_sprite_manager->ReplaceComponent(
            entity_instance.m_entity, entity_instance.m_data.m_sprite.value());
    }
    if (entity_instance.m_data.m_transform) {
        m_transform_manager->ReplaceComponent(
            entity_instance.m_entity,
            entity_instance.m_data.m_transform.value());
    }
    if (entity_instance.m_data.m_relationship) {
        m_relationship_manager->ReplaceComponent(
            entity_instance.m_entity,
            entity_instance.m_data.m_relationship.value());
    }
    if (entity_instance.m_data.m_tilemap) {
        m_tilemap_component_manager->ReplaceComponent(
            entity_instance.m_entity, entity_instance.m_data.m_tilemap);
    }
    if (entity_instance.m_data.m_animation) {
        m_animation_player_manager->RegisterEntity(
            entity_instance.m_entity,
            entity_instance.m_data.m_animation.value());
    }
}

void Context::RemoveEntity(Entity entity) {
    m_sprite_manager->RemoveEntity(entity);
    m_transform_manager->RemoveEntity(entity);
    m_relationship_manager->RemoveEntity(entity);
}