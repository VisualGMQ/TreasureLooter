#define SDL_MAIN_USE_CALLBACKS
#include "context.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    AnimationEditorContext::Init();
    CommonContext::ChangeContext(ANIMATION_EDITOR_CONTEXT);
    AnimationEditorContext::GetInst().InitSystem();
    AnimationEditorContext::GetInst().Initialize(argc, argv);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto& ctx = ANIMATION_EDITOR_CONTEXT;
    if (ctx.ShouldExit()) {
        return SDL_APP_SUCCESS;
    }
    ctx.Update();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    ANIMATION_EDITOR_CONTEXT.HandleEvents(*event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    AnimationEditorContext::GetInst().Shutdown();
    AnimationEditorContext::GetInst().ShutdownSystem();
    AnimationEditorContext::Destroy();
}
