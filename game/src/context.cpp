#include "context.hpp"
#include "log.hpp"
#include "rapidxml_print.hpp"
#include "relationship.hpp"
#include "sdl_call.hpp"
#include "serialize.hpp"
#include "sprite.hpp"
#include "transform.hpp"
#include "schema/serialize/sprite.hpp"

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
    logicUpdate();
    renderUpdate();
}

void Context::HandleEvents(const SDL_Event& event) {
    m_inspector->HandleEvents(event);
    if (event.type == SDL_EVENT_QUIT) {
        m_should_exit = true;
    }
}

bool Context::ShouldExit() {
    return m_should_exit;
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

    ////// this is a test //////
    Entity entity = createEntity();
    m_relationship_manager->Get(m_root_entity)->m_children.push_back(entity);
    m_transform_manager->RegisterEntity(m_root_entity);

    Sprite sprite;
    sprite.m_image =
        m_image_manager->Load("assets/Characters/Statue/SpriteSheet.png");
    auto tile_size = sprite.m_image->GetSize() / Vec2{4, 7};
    sprite.m_region.m_size = tile_size;
    sprite.m_size = tile_size * 3;
    m_sprite_manager->RegisterEntity(entity, std::move(sprite));
    m_transform_manager->RegisterEntity(entity, Transform{Pose{{200, 300}}, Pose{}});
}

void Context::logicUpdate() {
    m_relationship_manager->Update();
}

void Context::renderUpdate() {
    m_inspector->BeginFrame();
    m_renderer->Clear();

    m_sprite_manager->Update();
    m_inspector->Update();

    m_inspector->EndFrame();
    m_renderer->Present();
}

Entity Context::createEntity() {
    return m_last_entity++;
}