#pragma once

#include "animation.hpp"
#include "asset_table.hpp"
#include "audio.hpp"
#include "controller/controller.hpp"
#include "debugger.hpp"
#include "event.hpp"
#include "font.hpp"
#include "gameobject.hpp"
#include "input/finger.hpp"
#include "input/game_controller.hpp"
#include "input/keyboard.hpp"
#include "input/mouse.hpp"
#include "pch.hpp"
#include "physics_scene.hpp"
#include "prefab.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "tilemap.hpp"
#include "timer.hpp"
#include "window.hpp"

namespace tl {
class RoleConfigManager;

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
    std::unique_ptr<AnimationManager> animMgr;
    std::unique_ptr<DebugManager> debugMgr;
    std::unique_ptr<SceneManager> sceneMgr;
    std::unique_ptr<TileMapManager> tilemapMgr;
    std::unique_ptr<RoleConfigManager> roleConfigMgr;
    std::unique_ptr<AssetTable> assetTbl;
    std::unique_ptr<input::Keyboard> keyboard;
    std::unique_ptr<input::Mouse> mouse;
    std::unique_ptr<input::GameControllerManager> gameCtrlMgr;
    std::unique_ptr<input::FingerManager> fingerMgr;
    std::unique_ptr<controller::ControllerManager> controllerMgr;
    std::unique_ptr<Time> time;
    std::unique_ptr<TimerManager> timerMgr;
    std::unique_ptr<FontManager> fontMgr;
    std::unique_ptr<AudioManager> audioMgr;
    std::unique_ptr<PhysicsScene> physicsScene;
    std::unique_ptr<PrefabManager> prefabMgr;
    GameObjectID cameraGOID;
    Camera& GetCamera();
    const Camera& GetCamera() const;

    void Update();

    void Exit() { shouldExit_ = true; }
    bool ShouldExit() const { return shouldExit_; }

    Scene& GetCurScene();
    const Scene& GetCurScene() const;

private:
    SDL_Event event_;
    bool shouldExit_ = false;
    Camera defaultCamera_;

    ~Context();

    void initSDL();
    void quitSDL();
    void initImGui();
    void quitImGui();
    void postInit();
    void handleEvent();

    template <typename Level>
    void registerLevel2Scene(const std::string& sceneName) {
        auto scene = sceneMgr->Find(sceneName);
        TL_RETURN_IF_FALSE(scene);
        scene->RegisterLevel(std::make_unique<Level>(*scene));
    }

    static Context* inst;
};

}  // namespace tl
