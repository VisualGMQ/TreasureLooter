#pragma once

#include "animation.hpp"
#include "debugger.hpp"
#include "gameobject.hpp"
#include "pch.hpp"
#include "renderer.hpp"
#include "sprite.hpp"
#include "texture.hpp"
#include "window.hpp"

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
    GameObjectManager goMgr;
    AnimationManager animMgr;
    DebugManager debugMgr;

    void Update();

private:
    SDL_Event event;
    bool shouldExit = false;

    Context();

    void initSDL();
    void quitSDL();
    void drawSprite(GameObject&);
    void syncAnim2GO(GameObject&);
    void updateGO(GameObject* parent, GameObject* go);

    static Context* inst;
};

}  // namespace tl
