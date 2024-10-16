#include "context.hpp"
#include "asset_table.hpp"
#include "flags.hpp"
#include "log.hpp"
#include "math.hpp"
#include "renderer.hpp"
#include "transform.hpp"
#include "controller/touch_controller.hpp"

namespace tl {

Context* Context::inst = nullptr;

void Context::Init() {
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
    gameController = std::make_unique<GameController>();
    textureMgr = std::make_unique<TextureManager>();
    tilemapMgr = std::make_unique<TileMapManager>();
    fontMgr = std::make_unique<FontManager>();
    animMgr = std::make_unique<AnimationManager>();
    audioMgr = std::make_unique<AudioManager>();
    goMgr = std::make_unique<GameObjectManager>();
    assetTbl = std::make_unique<AssetTable>();
    sceneMgr = std::make_unique<SceneManager>();
    keyboard = std::make_unique<input::Keyboard>();
    mouse = std::make_unique<input::Mouse>();
    gameCtrlMgr = std::make_unique<input::GameControllerManager>();
    fingerMgr = std::make_unique<input::FingerManager>();
    debugMgr = std::make_unique<DebugManager>();
}

void Context::initSDL() {
    SDL_Init(SDL_INIT_EVERYTHING);
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
    IMG_Init(IMG_INIT_PNG);
    Mix_Init(MIX_INIT_MP3|MIX_INIT_OGG);
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

    audioMgr.reset();
    fontMgr.reset();
    timerMgr.reset();
    time.reset();
    gameController.reset();
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
    goMgr.reset();
    textureMgr.reset();
    renderer.reset();
    window.reset();

    quitSDL();
}

void Context::Update() {
    time->BeginRecordElapse();

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

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    renderer->Clear(Color{0.3, 0.3, 0.3});

    controllerMgr->Update();
    gameController->Update();
    keyboard->Update();
    mouse->Update();
    gameCtrlMgr->Update();
    fingerMgr->Update();
    sceneMgr->Update();
    debugMgr->Update();
    timerMgr->Update(time->GetElapse());

    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    renderer->SetScale(
            {io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y});
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(),
            renderer->renderer_);

    renderer->Present();
    sceneMgr->PostUpdate();

    time->EndRecordElapse();
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
