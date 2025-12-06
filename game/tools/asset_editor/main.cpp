#define SDL_MAIN_USE_CALLBACKS
#include "context.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "engine/sdl_call.hpp"
#include "spdlog/spdlog.h"
#include "lyra/lyra.hpp"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    // TODO: using lyra
    AssetEditorContext::Init();
    CommonContext::ChangeContext(ASSET_VIEWER_CONTEXT);
    AssetEditorContext::GetInst().InitSystem();
    AssetEditorContext::GetInst().Initialize();
    LOGI("asset viewer start");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto &ctx = ASSET_VIEWER_CONTEXT;
    if (ctx.ShouldExit()) {
        return SDL_APP_SUCCESS;
    }
    ASSET_VIEWER_CONTEXT.Update();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    ASSET_VIEWER_CONTEXT.HandleEvents(*event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    AssetEditorContext::GetInst().Shutdown();
    AssetEditorContext::GetInst().ShutdownSystem();
    AssetEditorContext::Destroy();
}
