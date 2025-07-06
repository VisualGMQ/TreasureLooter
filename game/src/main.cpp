#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "context.hpp"
#include "sdl_call.hpp"
#include "spdlog/spdlog.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    Context::Init();
    LOGI("app start");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto &ctx = Context::GetInst();
    if (ctx.ShouldExit()) {
        return SDL_APP_SUCCESS;
    }
    Context::GetInst().Update();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    Context::GetInst().HandleEvents(*event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    Context::Destroy();
}