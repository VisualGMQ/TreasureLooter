#pragma once

#include <luacode.h>

// Luau compile options used for every script/module compiled by the game.
// When the Luau debugger is enabled the debug level is raised to 2 so that
// local & upvalue names are available to the DAP server.
inline lua_CompileOptions MakeLuauCompileOptions() {
    lua_CompileOptions options{};
    options.optimizationLevel = 1;
    options.typeInfoLevel = 0;
    options.coverageLevel = 0;
#ifdef TL_ENABLE_LUAU_DEBUGGER
    options.debugLevel = 2;
#else
    options.debugLevel = 1;
#endif
    return options;
}
