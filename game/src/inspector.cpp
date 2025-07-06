#include "inspector.hpp"
#include "SDL3/SDL.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "imgui.h"
#include "renderer.hpp"
#include "window.hpp"

Inspector::Inspector(Window& window, Renderer& renderer)
    : m_window{window}, m_renderer{renderer} {
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(
        main_scale);  // Bake a fixed style scale. (until we have a solution for
                      // dynamic style scaling, changing this requires resetting
                      // Style + calling this again)
    style.FontScaleDpi =
        main_scale;  // Set initial font scale. (using
                     // io.ConfigDpiScaleFonts=true makes this unnecessary. We
                     // leave both here for documentation purpose)

    ImGui_ImplSDL3_InitForSDLRenderer(window.GetWindow(),
                                      renderer.GetRenderer());
    ImGui_ImplSDLRenderer3_Init(renderer.GetRenderer());
}

Inspector::~Inspector() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void Inspector::BeginFrame() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void Inspector::EndFrame() {
    ImGui::Render();
    auto& io = ImGui::GetIO();
    SDL_SetRenderScale(m_renderer.GetRenderer(), io.DisplayFramebufferScale.x,
                       io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                          m_renderer.GetRenderer());
}

void Inspector::Update() {
    ImGui::ShowDemoWindow();
}

void Inspector::HandleEvents(const SDL_Event& event) {
    ImGui_ImplSDL3_ProcessEvent(&event);
}
