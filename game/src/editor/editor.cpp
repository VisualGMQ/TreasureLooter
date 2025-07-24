#include "editor/editor.hpp"

#include "dialog.hpp"
#include "imgui.h"
#include "schema/serialize/asset_extensions.hpp"

#include <array>

void Editor::Update() {
    if (ImGui::Begin("Editor", &m_open)) {
        static const std::array<const char*, 2> asset_types = {
            "InputConfig",
            "Prefab",
        };
        
        static const std::array<std::string_view, 2> asset_extensions = {
            InputConfig_AssetExtension,
            EntityInstance_AssetExtension,
        };

        if (ImGui::BeginCombo("asset type", "None",
                              ImGuiComboFlags_NoPreview)) {
            for (size_t i = 0; i < asset_types.size(); i++) {
                auto asset_type = asset_types[i];
                if (ImGui::Selectable(asset_type, false)) {
                    FileDialog file_dialog{FileDialog::Type::OpenFile};
                    file_dialog.SetTitle("Open Asset");
                    file_dialog.AddFilter(asset_type, asset_extensions[i].data());
                    file_dialog.Open();

                    auto& files = file_dialog.GetSelectedFiles();
                    if (files.empty()) {
                        break;
                    }

                    auto& filename = files[0];
                    // TODO: read asset and edit
                }
            }
            ImGui::EndCombo();
        }
    }
    ImGui::End();
}