#pragma once

#include "animation.hpp"
#include "debugger.hpp"
#include "gameobject.hpp"
#include "pch.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "sprite.hpp"
#include "texture.hpp"
#include "window.hpp"
#include "asset_table.hpp"

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
    std::unique_ptr<AssetTable> assetTbl;
    std::unique_ptr<SceneManager> sceneMgr;

    void Update();

    void Exit() { shouldExit_ = true; }

private:
    SDL_Event event_;
    bool shouldExit_ = false;

    ~Context();

    void initSDL();
    void quitSDL();
    void initImGui();
    void quitImGui();
    void postInit();

    static Context* inst;
};

}  // namespace tl
