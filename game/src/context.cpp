#include "context.hpp"
#include "asset_table.hpp"
#include "controller/touch_controller.hpp"
#include "flags.hpp"
#include "level/game/game.hpp"
#include "level/test/test_maze.hpp"
#include "level/test/test_physics.hpp"
#include "level/test/test_playground.hpp"
#include "level/test/test_trigger.hpp"
#include "log.hpp"
#include "math.hpp"
#include "profile.hpp"
#include "renderer.hpp"
#include "role_config.hpp"

namespace tl {

Context* Context::inst = nullptr;

void Context::Init() {
    PROFILE_FUNC();
    
    inst = new Context;
    inst->postInit();
}

void Context::Destroy() {
    delete inst;
}

void Context::postInit() {
    initSDL();

    window = std::make_unique<Window>("Treasure Looter", 1024, 720);
    renderer = std::make_unique<Renderer>(*window);
    if (!window || !renderer) {
        quitSDL();
        exit(1);
    }
    initImGui();

    time = std::make_unique<Time>();
    timerMgr = std::make_unique<TimerManager>();
    keyboard = std::make_unique<input::Keyboard>();
    mouse = std::make_unique<input::Mouse>();
    gameCtrlMgr = std::make_unique<input::GameControllerManager>();
    fingerMgr = std::make_unique<input::FingerManager>();
    controllerMgr = std::make_unique<controller::ControllerManager>();
    textureMgr = std::make_unique<TextureManager>();
    roleConfigMgr = std::make_unique<RoleConfigManager>();
    tilemapMgr = std::make_unique<TileMapManager>();
    fontMgr = std::make_unique<FontManager>();
    animMgr = std::make_unique<AnimationManager>();
    audioMgr = std::make_unique<AudioManager>();
    prefabMgr = std::make_unique<PrefabManager>();
    assetTbl = std::make_unique<AssetTable>();
    sceneMgr = std::make_unique<SceneManager>();
    keyboard = std::make_unique<input::Keyboard>();
    mouse = std::make_unique<input::Mouse>();
    physicsScene = std::make_unique<PhysicsScene>();
    gameCtrlMgr = std::make_unique<input::GameControllerManager>();
    fingerMgr = std::make_unique<input::FingerManager>();
    debugMgr = std::make_unique<DebugManager>();

    registerLevel2Scene<TestLevel>("test/test-playground");
    registerLevel2Scene<TestPhysicsLevel>("test/test-sweep");
    registerLevel2Scene<TestMazeLevel>("test/test-maze");
    registerLevel2Scene<TestTriggerLevel>("test/test-trigger");
    registerLevel2Scene<GameLevel>("game/game");
}

void Context::initSDL() {
    SDL_Init(SDL_INIT_EVERYTHING);
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
    IMG_Init(IMG_INIT_PNG);
    Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) != 0) {
        LOGE("open audio device failed");
    }
    TTF_Init();

#ifdef TL_ANDROID
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif
}

void Context::quitSDL() {
    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

Context::~Context() {
    quitImGui();

    physicsScene.reset();
    prefabMgr.reset();
    audioMgr.reset();
    fontMgr.reset();
    timerMgr.reset();
    time.reset();
    controllerMgr.reset();
    fingerMgr.reset();
    gameCtrlMgr.reset();
    mouse.reset();
    keyboard.reset();
    sceneMgr.reset();
    tilemapMgr.reset();
    assetTbl.reset();
    debugMgr.reset();
    animMgr.reset();
    textureMgr.reset();
    roleConfigMgr.reset();
    renderer.reset();
    window.reset();

    quitSDL();
}

Camera& Context::GetCamera() {
    GameObject* go = sceneMgr->GetCurScene().GetGOMgr().Find(cameraGOID);
    if (go) {
        return go->camera;
    }
    return defaultCamera_;
}

const Camera& Context::GetCamera() const {
    GameObject* go = sceneMgr->GetCurScene().GetGOMgr().Find(cameraGOID);
    if (go && go->camera) {
        return go->camera;
    }
    return defaultCamera_;
}

void Context::Update() {
    time->BeginRecordElapse();
    PROFILE_FUNC();

    handleEvent();

    PROFILE_SECTION_BEGIN("imgui prepare");
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    // ImGui::ShowDemoWindow();
    PROFILE_SECTION_END();

    renderer->Clear(Color{0.3, 0.3, 0.3});

    sceneMgr->Update();
    controllerMgr->Update();
    keyboard->Update();
    mouse->Update();
    gameCtrlMgr->Update();
    fingerMgr->Update();
    debugMgr->Update();
    timerMgr->Update(time->GetElapse());

    renderer->Update();

    PROFILE_SECTION_BEGIN("imgui render");
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    renderer->SetScale(
        {io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y});
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(),
                                          renderer->renderer_);
    PROFILE_SECTION_END();

    renderer->Present();
    sceneMgr->PostUpdate();

    time->WaitForFps();
    time->EndRecordElapse();
}

Scene& Context::GetCurScene() {
    return sceneMgr->GetCurScene();
}

const Scene& Context::GetCurScene() const {
    return sceneMgr->GetCurScene();
}

void Context::handleEvent() {
    PROFILE_FUNC();
    while (SDL_PollEvent(&event_)) {
        ImGui_ImplSDL2_ProcessEvent(&event_);
        if (event_.type == SDL_QUIT) {
            shouldExit_ = true;
        }
        keyboard->HandleEvent(event_);
        mouse->HandleEvent(event_);
        gameCtrlMgr->HandleEvent(event_);
        fingerMgr->HandleEvent(event_);
        controllerMgr->HandleEvent(event_);
    }
}

void Context::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window->window_, renderer->renderer_);
    ImGui_ImplSDLRenderer2_Init(renderer->renderer_);
}

void Context::quitImGui() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

}  // namespace tl
