#include "context.hpp"
#include "log.hpp"
#include "math.hpp"
#include "renderer.hpp"
#include "flags.hpp"

namespace tl {

Context* Context::inst = nullptr;

void Context::Init() {
    inst = new Context;
}

void Context::Destroy() {
    delete inst;
}

Context::Context() : window{"Treasure Looter", 1024, 720}, renderer{window} {
    if (!window || !renderer) {
        quitSDL();
        exit(1);
    }
}

void Context::initSDL() {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    Mix_Init(MIX_INIT_MP3);
    TTF_Init();
}

void Context::quitSDL() {
    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Context::Update() {
    while (!shouldExit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldExit = true;
            }
        }

        renderer.Clear(Color{100, 100, 100});
        renderer.Present();
    }
}

}  // namespace tl
