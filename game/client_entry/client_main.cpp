#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "common/context.hpp"
#include "common/sdl_call.hpp"
#include "client/context.hpp"
#include "spdlog/spdlog.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    ClientContext::Init();
    CommonContext::ChangeContext(CLIENT_CONTEXT);
    ClientContext::GetInst().InitSystem();
    ClientContext::GetInst().Initialize(argc, argv);
    LOGI("app start");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto &ctx = CLIENT_CONTEXT;
    if (ctx.ShouldExit()) {
        return SDL_APP_SUCCESS;
    }
    CLIENT_CONTEXT.Update();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    CLIENT_CONTEXT.HandleEvents(*event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    ClientContext::GetInst().Shutdown();
    ClientContext::GetInst().ShutdownSystem();
    ClientContext::Destroy();
}
