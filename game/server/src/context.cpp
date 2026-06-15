#include "server/context.hpp"
#include "common/asset_manager.hpp"
#include "common/bind_point.hpp"
#include "common/cct.hpp"
#include "common/context.hpp"
#include "common/debug_drawer.hpp"
#include "common/event.hpp"
#include "common/log.hpp"
#include "common/net/udp.hpp"
#include "common/profile.hpp"
#include "common/relationship.hpp"
#include "common/scene.hpp"
#include "common/script/script.hpp"
#include "common/script/script_binding.hpp"
#include "common/sdl_call.hpp"
#include "common/serialize.hpp"
#include "common/static_collision.hpp"
#include "common/storage.hpp"
#include "common/tilemap.hpp"
#include "common/transform.hpp"
#include "common/trigger.hpp"
#include "common/uuid.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "schema/asset_info.hpp"
#include "schema/config.hpp"
#include "schema/proto/proto_binding.hpp"
#include "schema/proto/proto_event_binding.hpp"
#include "schema/serialize/input.hpp"
#include "schema/serialize/prefab.hpp"
#include "server/asset_manager.hpp"
#include "server/scene.hpp"
#include "server/script_binding.hpp"

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
    m_scene_manager = std::make_unique<ServerSceneManager>();
    m_script_binary_data_manager = std::make_unique<ScriptBinaryDataManager>();

    CommonContext::initGameConfig();

    m_assets_manager->GetManager<ScriptBinaryData>().Initialize(
        GetGameConfig());

    m_debug_drawer = std::unique_ptr<IDebugDrawer>(new TrivialDebugDrawer{});

    m_script_binary_data_manager->BindModule([](lua_State* L) {
        BindTLModule(L);
        BindServerModule(L);
    });

    m_time->SetFPS(60);
}

void ServerContext::HandleEvents(const SDL_Event& event) {
    CommonContext::HandleEvents(event);

    // SDL hijack Ctrl-C to SDL_EVENT_QUIT, so this will make Ctrl-C work again
    if (event.type == SDL_EVENT_QUIT) {
        Exit();
    }
}

void ServerContext::Update() {
    PROFILE_FRAME_NAMED("server_main_loop");

    m_time->Begin();

    auto elapse_time = m_time->GetElapseTime();

    PROFILE_SECTION();

    if (m_net_host) {
        m_net_host->HandleIncomingNetPacket();
    }

    m_time->Update();

    m_script_component_manager->Update();

    m_relationship_manager->Update();
    m_bind_point_component_manager->Update();
    m_static_collision_manager->Update();
    m_trigger_component_manager->Update();
    m_net_host->Flush();
    m_event_system->Update();
    m_timer_manager->Update(elapse_time);

    m_scene_manager->PoseUpdate();

    m_time->End();
}

void ServerContext::Shutdown() {
    m_script_component_manager->Clear();
    m_scene_manager->Switch({});

    CommonContext::Shutdown();
}

void ServerContext::NetListen(const NetAddress& address, int peer_count) {
    m_net_host = std::make_unique<UDPHost>(&address, peer_count);
}
