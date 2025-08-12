#include "inspector.hpp"

#include "SDL3/SDL.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "context.hpp"
#include "imgui.h"
#include "imgui_id_generator.hpp"
#include "level.hpp"
#include "relationship.hpp"
#include "renderer.hpp"
#include "sprite.hpp"
#include "transform.hpp"
#include "window.hpp"

#include "schema/display/physics.hpp"
#include "schema/display/prefab.hpp"
#include "schema/display/relationship.hpp"
#include "schema/display/sprite.hpp"

#ifdef TL_ENABLE_EDITOR

Inspector::Inspector(Window& window, Renderer& renderer)
    : m_window{window}, m_renderer{renderer} {}

void Inspector::Update() {
    if (ImGui::Begin("Debug Panel")) {
        bool physics_debug_draw =
            GAME_CONTEXT.m_physics_scene->IsEnableDebugDraw();
        if (ImGui::Checkbox("physics debug draw", &physics_debug_draw)) {
            GAME_CONTEXT.m_physics_scene->ToggleDebugDraw();
        }

        bool cct_debug_draw = GAME_CONTEXT.m_cct_manager->IsEnableDebugDraw();
        if (ImGui::Checkbox("cct debug draw", &cct_debug_draw)) {
            GAME_CONTEXT.m_cct_manager->ToggleDebugDraw();
        }
    }
    ImGui::End();

    if (ImGui::Begin("Entity Hierarchy", &m_hierarchy_window_open)) {
        auto level = GAME_CONTEXT.m_level_manager->GetCurrentLevel();
        if (level) {
            showEntityHierarchy(level->GetRootEntity());
        }
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

void Inspector::showEntityDetail(Entity entity) {
    auto& ctx = GAME_CONTEXT;
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

    if (ctx.m_animation_player_manager->Has(entity)) {
        auto value = ctx.m_animation_player_manager->Get(entity);
        InstanceDisplay("animation", *value);
    }

    if (ctx.m_cct_manager->Has(entity)) {
        auto value = ctx.m_cct_manager->Get(entity);
        InstanceDisplay("cct", *value);
    }
}

void Inspector::showEntityHierarchy(Entity node) {
    auto& ctx = GAME_CONTEXT;

    auto relationship = ctx.m_relationship_manager->Get(node);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick;
    flags |= relationship ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf;
    if (m_selected_entity && node == m_selected_entity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (ImGui::TreeNodeEx(
            ("Entity " + std::to_string(static_cast<uint32_t>(node))).c_str(),
            flags)) {
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

#endif