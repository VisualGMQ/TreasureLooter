#define SDL_MAIN_USE_CALLBACKS
#include "context.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "engine/sdl_call.hpp"
#include "spdlog/spdlog.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    editor::EditorContext::Init();
    CommonContext::ChangeContext(EDITOR_CONTEXT);
    editor::EditorContext::GetInst().InitSystem();
    editor::EditorContext::GetInst().Initialize();
    LOGI("editor start");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto &ctx = EDITOR_CONTEXT;
    if (ctx.ShouldExit()) {
        return SDL_APP_SUCCESS;
    }
    EDITOR_CONTEXT.Update();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    EDITOR_CONTEXT.HandleEvents(*event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    editor::EditorContext::GetInst().ShutdownSystem();
    editor::EditorContext::GetInst().Shutdown();
    editor::EditorContext::Destroy();
}