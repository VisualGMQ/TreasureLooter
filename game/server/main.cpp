#define SDL_MAIN_USE_CALLBACKS
#include "context.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "engine/sdl_call.hpp"
#include "spdlog/spdlog.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    ServerContext::Init();
    CommonContext::ChangeContext(SERVER_CONTEXT);
    ServerContext::GetInst().InitSystem();
    ServerContext::GetInst().Initialize();
    LOGI("server start");

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
    LOGI("server quit");
    ServerContext::GetInst().ShutdownSystem();
    ServerContext::GetInst().Shutdown();
    ServerContext::Destroy();
}