#include "inspector.hpp"

#include "SDL3/SDL.h"
#include "context.hpp"
#include "engine/imgui_id_generator.hpp"
#include "engine/level.hpp"
#include "engine/relationship.hpp"
#include "engine/renderer.hpp"
#include "engine/sprite.hpp"
#include "engine/transform.hpp"
#include "engine/window.hpp"
#include "imgui.h"

#include "engine/dialog.hpp"
#include "instance_display.hpp"
#include "schema/display/display.hpp"
#include "schema/prefab.hpp"
#include "schema/serialize/serialize.hpp"

Inspector::Inspector(CommonContext& context) : m_context{context} {}

void Inspector::Update() {
    if (m_hierarchy_window_open &&
        ImGui::Begin("Entity Hierarchy", &m_hierarchy_window_open,
                     ImGuiWindowFlags_MenuBar)) {
        auto level = m_context.m_level_manager->GetCurrentLevel();
        if (level) {
            showEntityHierarchy(level->GetRootEntity());
            showEntityHierarchy(level->GetUIRootEntity());
        }
    }
    if (m_hierarchy_window_open) {
        ImGui::End();
    }

    if (m_detail_window_open && ImGui::Begin("Detail", &m_detail_window_open)) {
        if (m_selected_entity) {
            showEntityDetail(m_selected_entity.value());
        }
    }
    if (m_detail_window_open) {
        ImGui::End();
    }

    ImGuiIDGenerator::Reset();
}

void Inspector::showEntityDetail(Entity entity) {
    if (m_context.m_transform_manager->Has(entity)) {
        auto value = m_context.m_transform_manager->Get(entity);
        InstanceDisplay("transform", *value);
    }

    if (m_context.m_relationship_manager->Has(entity)) {
        auto value = m_context.m_relationship_manager->Get(entity);
        InstanceDisplay("relationship", *value);
    }

    if (m_context.m_sprite_manager->Has(entity)) {
        auto value = m_context.m_sprite_manager->Get(entity);
        InstanceDisplay("sprite", *value);
    }

    if (m_context.m_animation_player_manager->Has(entity)) {
        auto value = m_context.m_animation_player_manager->Get(entity);
        InstanceDisplay("animation", *value);
    }

    if (m_context.m_cct_manager->Has(entity)) {
        auto value = m_context.m_cct_manager->Get(entity);
        InstanceDisplay("cct", *value);
    }
    if (m_context.m_trigger_component_manager->Has(entity)) {
        auto value = m_context.m_trigger_component_manager->Get(entity);
        InstanceDisplay("trigger", *value);
    }
    // if (m_context.m_ui_manager->Has(entity)) {
    //     auto value = m_context.m_ui_manager->Get(entity);
    //     InstanceDisplay("ui", *value);
    // }
}

void Inspector::showEntityHierarchy(Entity node) {
    auto relationship = m_context.m_relationship_manager->Get(node);

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