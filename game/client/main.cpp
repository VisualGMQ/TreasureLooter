#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "client/context.hpp"
#include "engine/sdl_call.hpp"
#include "spdlog/spdlog.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    GameContext::Init();
    CommonContext::ChangeContext(GAME_CONTEXT);
    GameContext::GetInst().InitSystem();
    GameContext::GetInst().Initialize();
    LOGI("app start");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto &ctx = GAME_CONTEXT;
    if (ctx.ShouldExit()) {
        return SDL_APP_SUCCESS;
    }
    GAME_CONTEXT.Update();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    GAME_CONTEXT.HandleEvents(*event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    GameContext::GetInst().Shutdown();
    GameContext::GetInst().ShutdownSystem();
    GameContext::Destroy();
}