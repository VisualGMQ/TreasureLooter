#pragma once
#include "common/context.hpp"

class NetAddress;

class ServerContext: public CommonContext {
public:
    static void Init();
    static void Destroy();
    static ServerContext& GetInst();

    ServerContext(const ServerContext&) = delete;
    ServerContext& operator=(const ServerContext&) = delete;
    ServerContext(ServerContext&&) = delete;
    ServerContext& operator=(ServerContext&&) = delete;

    void Initialize(int argc, char** argv) override;
    void HandleEvents(const SDL_Event& event) override;
    void Update() override;
    void Shutdown() override;

    void NetListen(const NetAddress&, int peer_count);

private:
    using CommonContext::CommonContext;

    static std::unique_ptr<ServerContext> instance;
};

#define SERVER_CONTEXT ServerContext::GetInst()
