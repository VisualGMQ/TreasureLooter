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

void Inspector::Update() {
    // if (ImGui::Begin("Debug Panel")) {
    //     bool physics_debug_draw =
    //         EDITOR_CONTEXT.m_physics_scene->IsEnableDebugDraw();
    //     if (ImGui::Checkbox("physics debug draw", &physics_debug_draw)) {
    //         EDITOR_CONTEXT.m_physics_scene->ToggleDebugDraw();
    //     }
    // }
    // ImGui::End();

    if (m_hierarchy_window_open &&
        ImGui::Begin("Entity Hierarchy", &m_hierarchy_window_open,
                     ImGuiWindowFlags_MenuBar)) {
        showMenu();

        auto level = EDITOR_CONTEXT.m_level_manager->GetCurrentLevel();
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
    auto& ctx = EDITOR_CONTEXT;
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
    if (ctx.m_trigger_component_manager->Has(entity)) {
        auto value = ctx.m_trigger_component_manager->Get(entity);
        InstanceDisplay("trigger", *value);
    }
}

void Inspector::showMenu() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Level")) {
            if (ImGui::MenuItem("New Level")) {
                FileDialog dialog{FileDialog::Type::SaveFile};
                dialog.SetTitle("Create Level");
                dialog.AddFilter(
                    AssetInfoManager::GetName<LevelContent>().data(),
                    AssetInfoManager::GetExtensionNoDot<LevelContent>().data());
                dialog.Open();

                auto files = dialog.GetSelectedFiles();
                if (!files.empty()) {
                    auto& filename = files.front();
                    AppendExtension(
                        filename,
                        AssetInfoManager::GetExtension<LevelContent>().data());
                    LevelHandle level =
                        EDITOR_CONTEXT.m_level_manager->Load(filename);
                    EDITOR_CONTEXT.m_level_manager->Switch(level);
                }
            }

            if (ImGui::MenuItem("Open Level")) {
                FileDialog dialog{FileDialog::Type::OpenFile};
                dialog.SetTitle("Open Level");
                dialog.AddFilter(
                    AssetInfoManager::GetName<LevelContent>().data(),
                    AssetInfoManager::GetExtensionNoDot<LevelContent>().data());
                dialog.Open();

                auto files = dialog.GetSelectedFiles();
                if (!files.empty()) {
                    LevelHandle level =
                        EDITOR_CONTEXT.m_level_manager->Load(files[0], true);
                    EDITOR_CONTEXT.m_level_manager->Switch(level);
                }
            }

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Instantiate")) {
            FileDialog dialog{FileDialog::Type::OpenFile};
            dialog.SetTitle("Choose Prefab");
            dialog.AddFilter(
                AssetInfoManager::GetName<Prefab>().data(),
                AssetInfoManager::GetExtensionNoDot<Prefab>().data());
            dialog.Open();

            auto files = dialog.GetSelectedFiles();
            if (!files.empty()) {
                auto& filename = files.front();

                LevelHandle current_level =
                    EDITOR_CONTEXT.m_level_manager->GetCurrentLevel();
                if (current_level) {
                    PrefabHandle handle =
                        EDITOR_CONTEXT.m_assets_manager->GetManager<Prefab>()
                            .Load(filename);
                    Entity entity = current_level->Instantiate(handle);
                    Entity root = current_level->GetRootEntity();
                    auto relationship =
                        EDITOR_CONTEXT.m_relationship_manager->Get(root);
                    relationship->m_children.push_back(entity);
                } else {
                    LOGE("no level");
                }
            }
        }
        ImGui::EndMenuBar();
    }
}

void Inspector::showEntityHierarchy(Entity node) {
    auto& ctx = EDITOR_CONTEXT;

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