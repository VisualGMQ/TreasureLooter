#include "context.hpp"
#include "log.hpp"
#include "sdl_call.hpp"

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
    m_window = std::make_unique<Window>("TreasureLooter", 1024, 720);
    m_renderer = std::make_unique<Renderer>(*m_window);
    m_renderer->SetClearColor({0.3, 0.3, 0.3, 1});

    m_image_manager = std::make_unique<ImageManager>(*m_renderer);

    m_inspector = std::make_unique<Inspector>(*m_window, *m_renderer);

    GameObject go;
    go.m_pose.m_position = {200, 300};
    go.m_sprite.m_image =
        m_image_manager->Load("assets/Characters/Statue/SpriteSheet.png");
    auto tile_size = go.m_sprite.m_image->GetSize() / Vec2{4, 7};
    go.m_sprite.m_region.m_size = tile_size;
    go.m_sprite.m_size = tile_size * 3;

    m_root.m_children.push_back(go);
}

void Context::drawSpriteRecursive(const GameObject& go) {
    if (go.m_sprite) {
        auto src_region = go.m_sprite.m_region;
        Region dst_region;
        auto& global_pose = go.GetGlobalPose();
        auto image_size = go.m_sprite.m_region.m_size;
        dst_region.m_topleft = global_pose.m_position - image_size * 0.5;
        dst_region.m_size = go.m_sprite.m_size * global_pose.m_scale;

        m_renderer->DrawImage(*go.m_sprite.m_image, src_region, dst_region,
                              global_pose.m_rotation.Value(),
                              dst_region.m_size * 0.5, go.m_sprite.m_flip);
    }

    for (auto& child : go.m_children) {
        drawSpriteRecursive(child);
    }
}

void Context::updateGOPoses() {
    for (auto& child : m_root.m_children) {
        updatePoseRecursive(m_root, child);
    }
}

void Context::updatePoseRecursive(const GameObject& parent, GameObject& child) {
    child.m_global_pose = parent.m_global_pose * child.m_pose;

    for (auto& c : child.m_children) {
        updatePoseRecursive(child, c);
    }
}

void Context::logicUpdate() {
    updateGOPoses();
}

void Context::renderUpdate() {
    m_inspector->BeginFrame();
    m_renderer->Clear();

    drawSpriteRecursive(m_root);

    m_inspector->Update();

    m_inspector->EndFrame();
    m_renderer->Present();
}