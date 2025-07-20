#include "context.hpp"
#include "log.hpp"
#include "rapidxml_utils.hpp"
#include "relationship.hpp"
#include "schema/prefab.hpp"
#include "schema/serialize/prefab.hpp"
#include "sdl_call.hpp"
#include "serialize.hpp"
#include "sprite.hpp"
#include "storage.hpp"
#include "transform.hpp"
#include "imgui.h"

#include <iostream>
#include <sstream>

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
    m_gamepad_manager.reset();
    m_touchs.reset();
    m_mouse.reset();
    m_keyboard.reset();
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
        Entity entity1;
        {
            auto prefab =
                LoadEntityInstanceAsset("assets/gpa/waggo.entity.xml");
            entity1 = prefab.m_entity;
            if (prefab.m_data.m_transform) {
                m_transform_manager->RegisterEntity(
                    prefab.m_entity, prefab.m_data.m_transform.value());
            }
            if (prefab.m_data.m_sprite) {
                m_sprite_manager->RegisterEntity(
                    prefab.m_entity, prefab.m_data.m_sprite.value());
            }
            if (prefab.m_data.m_relationship) {
                m_relationship_manager->RegisterEntity(
                    prefab.m_entity, prefab.m_data.m_relationship.value());
            }

            m_relationship_manager->Get(m_root_entity)
                ->m_children.push_back(prefab.m_entity);
        }
        {
            auto prefab =
                LoadEntityInstanceAsset("assets/gpa/waggo2.entity.xml");
            if (prefab.m_data.m_transform) {
                m_transform_manager->RegisterEntity(
                    prefab.m_entity, prefab.m_data.m_transform.value());
            }
            if (prefab.m_data.m_sprite) {
                m_sprite_manager->RegisterEntity(
                    prefab.m_entity, prefab.m_data.m_sprite.value());
            }
            if (prefab.m_data.m_relationship) {
                m_relationship_manager->RegisterEntity(
                    prefab.m_entity, prefab.m_data.m_relationship.value());
            }

            m_relationship_manager->RegisterEntity(
                entity1, Relationship{{prefab.m_entity}});
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
        m_touchs->HandleEvent(event.tfinger);
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

Context::Context() {
    SDL_CALL(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK |
                      SDL_INIT_GAMEPAD));
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    m_window = std::make_unique<Window>("TreasureLooter", 1024, 720);
    m_renderer = std::make_unique<Renderer>(*m_window);
    m_renderer->SetClearColor({0.3, 0.3, 0.3, 1});

    m_image_manager = std::make_unique<ImageManager>(*m_renderer);

    m_inspector = std::make_unique<Inspector>(*m_window, *m_renderer);

    m_root_entity = createEntity();

    m_transform_manager = std::make_unique<TransformManager>();
    m_sprite_manager = std::make_unique<SpriteManager>();
    m_relationship_manager =
        std::make_unique<RelationshipManager>(m_root_entity);
    m_transform_manager->RegisterEntity(m_root_entity);
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_touchs = std::make_unique<Touches>();
    m_gamepad_manager = std::make_unique<GamepadManager>();
}

void Context::logicUpdate() {
    m_gamepad_manager->Update();
    m_keyboard->Update();
    m_mouse->Update();
    m_touchs->Update();
    m_relationship_manager->Update();
}

void Context::gameLogicUpdate() {
    // TODO: game logic here
    auto children = m_relationship_manager->Get(m_root_entity);
    Entity entity = children->m_children[0];

    Transform* transform = m_transform_manager->Get(entity);

    auto& gamepads = m_gamepad_manager->GetGamepads();
    if (gamepads.empty()) {
        return;
    }

    auto& gamepad = gamepads.begin()->second;
    transform->m_position.x += 0.1f * gamepad.GetAxis(SDL_GAMEPAD_AXIS_LEFTX).Value();
    transform->m_position.y += 0.1f * gamepad.GetAxis(SDL_GAMEPAD_AXIS_LEFTY).Value();
    if (gamepad.GetButton(SDL_GAMEPAD_BUTTON_WEST).IsPressed()) {
        transform->m_rotation += 10;
    }
    if (gamepad.GetButton(SDL_GAMEPAD_BUTTON_EAST).IsReleased()) {
        transform->m_rotation -= 10;
    }
}

void Context::logicPostUpdate() {
    m_mouse->PostUpdate();
    m_touchs->PostUpdate();
}

void Context::renderUpdate() {
    m_inspector->BeginFrame();
    m_renderer->Clear();

    auto& fingers = m_touchs->GetFingers();
    if (ImGui::Begin("finger test")) {
        for (int i = 0; i < fingers.size(); ++i) {
            auto& finger = fingers[i];
            std::string status;
            if (finger.IsPressing()) {
                status = "Pressing";
            } else if (finger.IsPressed()) {
                status = "Pressed";
            } else if (finger.IsReleased()) {
                status = "Released";
            } else if (finger.IsReleasing()) {
                status = "Releasing";
            } else {
                status = "Unknown";
            }

            ImGui::Text("finger index: %d, status: %s, position: (%f, %f), "
                        "offset: (%f, %f)",
                        i, status.c_str(), finger.Position().x,
                        finger.Position().y, finger.Offset().x,
                        finger.Offset().y);
        }
        ImGui::End();
    }

    m_sprite_manager->Update();
    m_inspector->Update();

    m_inspector->EndFrame();
    m_renderer->Present();
}

Entity Context::createEntity() {
    return m_last_entity++;
}