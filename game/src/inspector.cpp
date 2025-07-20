#include "inspector.hpp"

#include "SDL3/SDL.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "context.hpp"
#include "imgui.h"
#include "imgui_id_generator.hpp"
#include "relationship.hpp"
#include "renderer.hpp"
#include "sprite.hpp"
#include "transform.hpp"
#include "window.hpp"

#include "schema/display/flip.hpp"
#include "schema/display/prefab.hpp"
#include "schema/display/relationship.hpp"
#include "schema/display/sprite.hpp"

Inspector::Inspector(Window& window, Renderer& renderer)
    : m_window{window}, m_renderer{renderer} {
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
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
    if (ImGui::Begin("Entity Hierarchy", &m_hierarchy_window_open)) {
        showEntityHierarchy(Context::GetInst().GetRootEntity());
    }
    ImGui::End();

    if (ImGui::Begin("Detail", &m_detail_window_open)) {
        if (m_selected_entity) {
            showEntityDetail(m_selected_entity.value());
        }
    }
    ImGui::End();

    ImGuiIDGenerator::Reset();
}

void Inspector::HandleEvents(const SDL_Event& event) {
    ImGui_ImplSDL3_ProcessEvent(&event);
}

void Inspector::showEntityDetail(Entity entity) {
    auto& ctx = Context::GetInst();
    if (ctx.m_transform_manager->Has(entity)) {
        auto value = ctx.m_transform_manager->Get(entity);
        InstanceDisplay("transform", *value);
    }

    if (ctx.m_relationship_manager->Has(entity)) {
        auto value = ctx.m_relationship_manager->Get(entity);
        InstanceDisplay("relationship", *value);
    }

    if (ctx.m_sprite_manager->Has(entity)) {
        auto value = ctx.m_sprite_manager->Get(entity);
        InstanceDisplay("sprite", *value);
    }
}

void Inspector::showEntityHierarchy(Entity node) {
    auto& ctx = Context::GetInst();

    auto relationship = ctx.m_relationship_manager->Get(node);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick;
    flags |= relationship ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf;
    if (m_selected_entity && node == m_selected_entity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (ImGui::TreeNodeEx(("Entity " + std::to_string(node)).c_str(), flags)) {
        if (ImGui::IsItemClicked()) {
            m_selected_entity = node;
        }
        if (relationship) {
            for (auto child : relationship->m_children) {
                showEntityHierarchy(child);
            }
        }

        ImGui::TreePop();
    }
}
