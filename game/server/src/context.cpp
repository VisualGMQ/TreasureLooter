#include "server/context.hpp"
#include "SDL3_ttf/SDL_ttf.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "common/uuid.hpp"
#include "common/asset_manager.hpp"
#include "common/bind_point.hpp"
#include "common/cct.hpp"
#include "common/context.hpp"
#include "common/debug_drawer.hpp"
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
#include "common/transform.hpp"
#include "common/trigger.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "schema/asset_info.hpp"
#include "schema/config.hpp"
#include "schema/serialize/input.hpp"
#include "schema/serialize/prefab.hpp"
#include "server/asset_manager.hpp"
#include "server/logic.hpp"
#include "server/scene.hpp"

#include <memory>

std::unique_ptr<ServerContext> ServerContext::instance;

void ServerContext::Init() {
    if (!instance) {
        instance = std::unique_ptr<ServerContext>(new ServerContext());
    } else {
        LOGW("inited context singleton twice!");
    }
}

void ServerContext::Destroy() {
    instance.reset();
}

ServerContext& ServerContext::GetInst() {
    return *instance;
}

void ServerContext::Initialize(int argc, char** argv) {
    PROFILE_SECTION();

    CommonContext::Initialize(argc, argv);
    m_assets_manager = std::make_unique<ServerAssetsManager>();
    m_scene_manager = std::unique_ptr<ServerSceneManager>(new ServerSceneManager{});
    m_script_binary_data_manager = std::make_unique<ScriptBinaryDataManager>();
    m_logic = std::make_unique<ServerLogic>();

    CommonContext::initGameConfig();

    m_assets_manager->GetManager<ScriptBinaryData>().Initialize(GetGameConfig());

    m_debug_drawer = std::unique_ptr<IDebugDrawer>(new TrivialDebugDrawer{});

    m_logic->OnInit();

    m_time->SetFPS(60);
}

void ServerContext::Update() {
    PROFILE_FRAME_NAMED("server_main_loop");

    m_time->Begin();

    auto elapse_time = m_time->GetElapseTime();

    PROFILE_SECTION();

    m_time->Update();

    m_script_component_manager->Update();
    m_logic->OnUpdate(elapse_time);

    m_relationship_manager->Update();
    m_bind_point_component_manager->Update();
    m_static_collision_manager->Update();
    m_trigger_component_manager->Update();
    m_event_system->Update();
    m_timer_manager->Update(elapse_time);

    m_scene_manager->PoseUpdate();

    m_time->End();
}

void ServerContext::Shutdown() {
    m_logic->OnQuit();
    m_logic.reset();
    CommonContext::Shutdown();
}
