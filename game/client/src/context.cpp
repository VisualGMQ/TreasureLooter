#include "client/context.hpp"

#include "engine/asset_manager.hpp"
#include "engine/profile.hpp"
#include "engine/relationship.hpp"

std::unique_ptr<GameContext> GameContext::instance;

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
    PROFILE_SECTION();
    
    CommonContext::Initialize();
    m_motor_manager = std::make_unique<MotorManager>();

    m_input_manager->Initialize(
        m_assets_manager->GetManager<InputConfig>().Load(
            GetGameConfig().m_input_config_asset),
        *this);

    m_level_manager->Switch(m_assets_manager->GetManager<Level>().Load(
        GetGameConfig().m_basic_level_asset));
}

void GameContext::Shutdown() {
    m_motor_manager.reset();
    CommonContext::Shutdown();
}

void GameContext::Update() {
    PROFILE_MAIN_FRAME();
    
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
