#include "context.hpp"
#include "enet.h"

#include "engine/asset_manager.hpp"
#include "engine/relationship.hpp"
#include "engine/sdl_call.hpp"

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

void ServerContext::InitSystem() {
    LOGT("system init");
    SDL_CALL(SDL_Init(SDL_INIT_EVENTS));
}

void ServerContext::ShutdownSystem() {
    SDL_Quit();
    LOGT("system shutdown");
}

void ServerContext::Initialize() {
    m_should_exit = false;
    m_is_inited = true;
    m_assets_manager = std::make_unique<AssetsManager>();

    m_transform_manager = std::make_unique<TransformManager>();
    m_relationship_manager = std::make_unique<RelationshipManager>();

    m_time = std::make_unique<Time>();
    m_physics_scene = std::make_unique<PhysicsScene>();
    m_cct_manager = std::make_unique<CCTManager>();
    m_event_system = std::make_unique<EventSystem>();
    m_level_manager = std::make_unique<LevelManager>();
    m_trigger_component_manager = std::make_unique<TriggerComponentManager>();
    m_timer_manager = std::make_unique<TimerManager>();
    m_motor_manager = std::make_unique<MotorManager>();
    m_bind_point_component_manager =
        std::make_unique<BindPointsComponentManager>();

    initNetwork();
}

void ServerContext::Shutdown() {
    shutdownNetwork();
    CommonContext::Shutdown();
}

void ServerContext::Update() {
    while (enet_host_service(m_enet_host, &m_enet_event, 1000) > 0) {
        char buf[1024] = {0};
        enet_address_get_host_ip(&m_enet_event.peer->address, buf,
                                 sizeof(buf));
        if (m_enet_event.type == ENET_EVENT_TYPE_CONNECT) {
            LOGI("new peer connected: {}:{}", buf,
                 m_enet_event.peer->address.port);
        } else if (m_enet_event.type == ENET_EVENT_TYPE_DISCONNECT ||
                   m_enet_event.type == ENET_EVENT_TYPE_DISCONNECT_TIMEOUT) {
            LOGI("peer disconnected: {}:{}", buf,
                 m_enet_event.peer->address.port);
        } else if (m_enet_event.type == ENET_EVENT_TYPE_RECEIVE) {
            ENetPacket* packet = m_enet_event.packet;
            LOGI("receive message from {}:{} - {}", buf, m_enet_event.peer->address.port, (char*)packet->data);
            enet_packet_destroy(packet);
        }
    }
}

void ServerContext::initNetwork() {
    if (enet_initialize() != 0) {
        LOGE("initialize enet failed!");
        Exit();
        return;
    }

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 1234;

    m_enet_host = enet_host_create(&address, 32, 2, 0, 0);
    if (!m_enet_host) {
        LOGE("create server host failed");
        Exit();
    }
}

void ServerContext::shutdownNetwork() {
    enet_host_destroy(m_enet_host);
    enet_deinitialize();
}