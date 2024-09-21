#pragma once

#include "pch.hpp"
#include "window.hpp"
#include "renderer.hpp"
#include "texture.hpp"

namespace tl {

class Context {
public:
    static Context& GetInst() {
        assert(inst);
        return *inst;
    }

    static void Init();
    static void Destroy();

    Window window;
    Renderer renderer;
    TextureManager textureMgr;

    void Update();

private:
    SDL_Event event;
    bool shouldExit = false;

    Context();

    void initSDL();
    void quitSDL();

    static Context* inst;
};

}
