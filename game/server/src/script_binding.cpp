#include "server/script_binding.hpp"
#include "common/context.hpp"
#include "common/net/udp.hpp"
#include "common/script/luabridge_include.hpp"
#include "common/script/script.hpp"
#include "server/context.hpp"

void BindServerModule(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL_Server")
            .deriveClass<ServerContext, CommonContext>("GameContext")
                .addFunction("NetListen",
                             +[](ServerContext* ctx, const NetAddress& addr,
                                 int peer_count) {
                                 ctx->NetListen(addr, peer_count);
                             })
                .addFunction("GetConfig",
                             +[](ServerContext* ctx) -> const ServerConfig* {
                                 return &ctx->GetConfig();
                             })
            .endClass()
            .addFunction("GetContext", +[]() -> ServerContext* {
                return &ServerContext::GetInst();
            })
        .endNamespace();
}
