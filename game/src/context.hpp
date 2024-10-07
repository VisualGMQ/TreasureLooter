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
#include "tilemap.hpp"
#include "input/keyboard.hpp"
#include "input/mouse.hpp"
#include "input/game_controller.hpp"
#include "input/finger.hpp"
#include "controller/controller.hpp"
#include "game_controller.hpp"

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
    std::unique_ptr<SceneManager> sceneMgr;
    std::unique_ptr<TileMapManager> tilemapMgr;
    std::unique_ptr<AssetTable> assetTbl;
    std::unique_ptr<input::Keyboard> keyboard;
    std::unique_ptr<input::Mouse> mouse;
    std::unique_ptr<input::GameControllerManager> gameCtrlMgr;
    std::unique_ptr<input::FingerManager> fingerMgr;
    std::unique_ptr<controller::ControllerManager> controllerMgr;
    std::unique_ptr<GameController> gameController;

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
