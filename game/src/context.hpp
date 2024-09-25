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

    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<TextureManager> textureMgr;
    std::unique_ptr<GameObjectManager> goMgr;
    std::unique_ptr<AnimationManager> animMgr;
    std::unique_ptr<DebugManager> debugMgr;

    void Update();

private:
    SDL_Event event;
    bool shouldExit = false;

    Context();
    ~Context();

    void initSDL();
    void quitSDL();
    void drawSprite(GameObject&);
    void syncAnim2GO(GameObject&);
    void updateGO(GameObject* parent, GameObject* go);

    void initImGui();
    void quitImGui();

    static Context* inst;
};

}  // namespace tl
