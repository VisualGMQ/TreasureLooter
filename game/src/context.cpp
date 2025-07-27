#include "context.hpp"
#include "imgui.h"
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
    } else {
        LOGW("inited context singleton twice!");
    }
}

void Context::Destroy() {
    instance.reset();
}

Context& Context::GetInst() {
    return *instance;
}

Context::~Context() {
    m_input_manager.reset();
    m_generic_assets_manager.reset();
    m_gamepad_manager.reset();
    
#ifdef TL_ENABLE_EDITOR
    m_editor.reset();
#endif
    
    m_touches.reset();
    m_mouse.reset();
    m_keyboard.reset();
    m_tilemap_component_manager.reset();
    m_tilemap_manager.reset();
    m_sprite_manager.reset();
    m_transform_manager.reset();
    m_inspector.reset();
    m_image_manager.reset();
    m_renderer.reset();
    m_window.reset();
    SDL_Quit();
    LOGI("game exits");
}

void Context::Update() {
    ////////// this is a test //////////
    static bool executed = false;

    if (!executed) {
        {
            auto result = LoadAsset<EntityInstance>("assets/gpa/waggo.prefab.xml");
            result.m_payload.m_entity = createEntity();
            RegisterEntity(result.m_payload);

            auto root_relationship = m_relationship_manager->Get(GetRootEntity());
            root_relationship->m_children.push_back(result.m_payload.m_entity);
        }
        {
            auto result =
                LoadAsset<EntityInstance>("assets/gpa/tilemap.prefab.xml");
            result.m_payload.m_entity = createEntity();
            RegisterEntity(result.m_payload);

            auto root_relationship =
                m_relationship_manager->Get(GetRootEntity());
            root_relationship->m_children.push_back(result.m_payload.m_entity);
        }
        executed = true;
    }
    ////////////////////////////////////

    logicUpdate();
    gameLogicUpdate();
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

Entity Context::GetRootEntity() {
    return m_root_entity;
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

    m_image_manager = std::make_unique<ImageManager>(*m_renderer);
    m_tilemap_manager = std::make_unique<TilemapManager>();
    m_tilemap_component_manager = std::make_unique<TilemapComponentManager>();

    m_inspector = std::make_unique<Inspector>(*m_window, *m_renderer);
    
#ifdef TL_ENABLE_EDITOR
    m_editor = std::make_unique<Editor>();
#endif

    m_root_entity = createEntity();

    m_transform_manager = std::make_unique<TransformManager>();
    m_sprite_manager = std::make_unique<SpriteManager>();
    m_relationship_manager =
        std::make_unique<RelationshipManager>(m_root_entity);
    m_transform_manager->RegisterEntity(m_root_entity);
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_touches = std::make_unique<Touches>();
    m_gamepad_manager = std::make_unique<GamepadManager>();

    m_generic_assets_manager = std::make_unique<GenericAssetsManager>();

    m_input_manager = std::make_unique<InputManager>(
        *this, std::string{"assets/gpa/input_config"} +
                   InputConfig_AssetExtension.data());

}

void Context::logicUpdate() {
    m_gamepad_manager->Update();
    m_keyboard->Update();
    m_mouse->Update();
    m_touches->Update();
    m_relationship_manager->Update();
}

void Context::gameLogicUpdate() {
    // TODO: game logic here
    auto children = m_relationship_manager->Get(m_root_entity);
    Entity entity = children->m_children[0];

    Transform* transform = m_transform_manager->Get(entity);

    auto& action = m_input_manager->GetAction("Attack");
    auto& x_axis = m_input_manager->GetAxis("MoveX");
    auto& y_axis = m_input_manager->GetAxis("MoveY");
    transform->m_position.x += 0.1f * x_axis.Value();
    transform->m_position.y += 0.1f * y_axis.Value();
    if (action.IsPressed()) {
        transform->m_rotation += 10;
    }
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

Entity Context::createEntity() {
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
            entity_instance.m_entity,
            entity_instance.m_data.m_tilemap);
    }
}

void Context::RemoveEntity(Entity entity) {
    m_sprite_manager->RemoveEntity(entity);
    m_transform_manager->RemoveEntity(entity);
    m_relationship_manager->RemoveEntity(entity);
}