#include "common/context.hpp"

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
};

#define SERVER_CONTEXT ServerContext::GetInst()
