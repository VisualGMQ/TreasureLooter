#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "client/context.hpp"
#include "context.hpp"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    TilemapDetourDataBakerContext::Init();
    CommonContext::ChangeContext(TilemapDetourDataBakerContext::GetInst());
    TilemapDetourDataBakerContext::GetInst().InitSystem();
    TilemapDetourDataBakerContext::GetInst().Initialize(argc, argv);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto& ctx = TilemapDetourDataBakerContext::GetInst();
    if (ctx.ShouldExit()) {
        return SDL_APP_SUCCESS;
    }
    ctx.Update();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    TilemapDetourDataBakerContext::GetInst().HandleEvents(*event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    TilemapDetourDataBakerContext::GetInst().Shutdown();
    TilemapDetourDataBakerContext::GetInst().ShutdownSystem();
    TilemapDetourDataBakerContext::Destroy();
}
