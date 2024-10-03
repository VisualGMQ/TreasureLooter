#include "context.hpp"
#include "flags.hpp"
#include "log.hpp"
#include "math.hpp"
#include "renderer.hpp"
#include "transform.hpp"
#include "asset_table.hpp"

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

    textureMgr = std::make_unique<TextureManager>();
    goMgr = std::make_unique<GameObjectManager>();
    animMgr = std::make_unique<AnimationManager>();
    debugMgr = std::make_unique<DebugManager>();
    assetTbl = std::make_unique<AssetTable>();
    sceneMgr = std::make_unique<SceneManager>();
}

void Context::initSDL() {
    SDL_Init(SDL_INIT_EVERYTHING);
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
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

Context::~Context() {
    quitImGui();

    sceneMgr.reset();
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
    while (!shouldExit_) {
        while (SDL_PollEvent(&event_)) {
            ImGui_ImplSDL2_ProcessEvent(&event_);
            if (event_.type == SDL_QUIT) {
                shouldExit_ = true;
            }
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        renderer->Clear(Color{100, 100, 100});

        sceneMgr->Update();
        debugMgr->Update();

        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO();
        renderer->SetScale({io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y});
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer->renderer_);

        renderer->Present();

        sceneMgr->PostUpdate();
    }
}


void Context::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

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
