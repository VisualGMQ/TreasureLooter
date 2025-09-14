#pragma once
#include "enet.h"
#include "engine/context.hpp"

class ServerContext: public CommonContext {
public:
    static void Init();
    static void Destroy();
    static ServerContext& GetInst();

    ServerContext(const ServerContext&) = delete;
    ServerContext& operator=(const ServerContext&) = delete;
    ServerContext(ServerContext&&) = delete;
    ServerContext& operator=(ServerContext&&) = delete;

    void InitSystem() override;
    void ShutdownSystem() override;
    void Initialize() override;
    void Shutdown() override;
    void Update() override;

private:
    static std::unique_ptr<ServerContext> instance;

    ENetEvent m_enet_event;
    ENetPeer m_enet_peer;
    ENetAddress m_enet_address;
    ENetHost* m_enet_host{};

    ServerContext() = default;
    void initNetwork();
    void shutdownNetwork();
};

#define SERVER_CONTEXT ServerContext::GetInst()