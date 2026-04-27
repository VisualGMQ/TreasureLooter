#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "common/context.hpp"
#include "common/sdl_call.hpp"
#include "spdlog/spdlog.h"
#include "server/context.hpp"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    ServerContext::Init();
    CommonContext::ChangeContext(SERVER_CONTEXT);
    ServerContext::GetInst().InitSystem();
    ServerContext::GetInst().Initialize(argc, argv);
    LOGI("app start");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto &ctx = SERVER_CONTEXT;
    if (ctx.ShouldExit()) {
        return SDL_APP_SUCCESS;
    }
    SERVER_CONTEXT.Update();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    SERVER_CONTEXT.HandleEvents(*event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    ServerContext::GetInst().Shutdown();
    ServerContext::GetInst().ShutdownSystem();
    ServerContext::Destroy();
}
