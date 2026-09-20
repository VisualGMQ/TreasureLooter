#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "common/context.hpp"
#include "context.hpp"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    BehaviorTreeEditorContext::Init();
    CommonContext::ChangeContext(BT_EDITOR_CONTEXT);
    BehaviorTreeEditorContext::GetInst().InitSystem();
    BehaviorTreeEditorContext::GetInst().Initialize(argc, argv);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto &ctx = BT_EDITOR_CONTEXT;
    if (ctx.ShouldExit()) {
        return SDL_APP_SUCCESS;
    }
    BT_EDITOR_CONTEXT.Update();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    BT_EDITOR_CONTEXT.HandleEvents(*event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    BehaviorTreeEditorContext::GetInst().Shutdown();
    BehaviorTreeEditorContext::GetInst().ShutdownSystem();
    BehaviorTreeEditorContext::Destroy();
}
