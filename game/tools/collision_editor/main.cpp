#define SDL_MAIN_USE_CALLBACKS
#include "context.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "engine/sdl_call.hpp"
#include "spdlog/spdlog.h"
#include "lyra/lyra.hpp"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    // TODO: using lyra
    CollisionEditorContext::Init();
    CommonContext::ChangeContext(COLLISION_EDITOR_CONTEXT);
    CollisionEditorContext::GetInst().InitSystem();
    CollisionEditorContext::GetInst().Initialize();
    LOGI("collision editor start");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto &ctx = COLLISION_EDITOR_CONTEXT;
    if (ctx.ShouldExit()) {
        return SDL_APP_SUCCESS;
    }
    COLLISION_EDITOR_CONTEXT.Update();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    COLLISION_EDITOR_CONTEXT.HandleEvents(*event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    CollisionEditorContext::GetInst().Shutdown();
    CollisionEditorContext::GetInst().ShutdownSystem();
    CollisionEditorContext::Destroy();
}
