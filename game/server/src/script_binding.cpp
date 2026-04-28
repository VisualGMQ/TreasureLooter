#include "server/script_binding.hpp"
#include "common/script/luabridge_include.hpp"

void BindServerModule(lua_State* L) {
    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL_Server")
        .endNamespace();
}

