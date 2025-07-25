#include "editor/editor.hpp"

#include "dialog.hpp"
#include "imgui.h"
#include "schema/display/input.hpp"
#include "schema/display/prefab.hpp"
#include "schema/serialize/asset_extensions.hpp"
#include "schema/serialize/input.hpp"
#include "schema/serialize/prefab.hpp"

#include <array>

struct AssetDisplay {
    template <typename T>
    void operator()(AssetLoadResult<T>& payload) {
        InstanceDisplay("Asset", payload.m_payload);
    }

    void operator()(std::monostate) const {}
};

template <typename T>
AssetTypes LoadAssetFromPath(const Path& filename) {
    auto result = LoadAsset<T>(filename);
    return AssetTypes{result};
}

struct AssetSaver {
    AssetSaver(const Path& filename) : m_filename(filename) {}
     
    template <typename T>
    void operator()(const AssetLoadResult<T>& payload) {
        SaveAsset(payload.m_uuid, payload.m_payload, m_filename);
    }

    void operator()(std::monostate) {}

private:
    Path m_filename;
};



template <typename T>
AssetTypes CreateAsset() {
    AssetLoadResult<T> result;
    result.m_uuid = UUID::CreateV4();
    return AssetTypes{result};
}

void Editor::Update() {
    if (ImGui::Begin("Editor", &m_open)) {
        static const std::array<const char*, 2> asset_types = {
            "InputConfig",
            "EntityInstance",
        };

        static const std::array<std::string_view, 2> asset_extensions = {
            InputConfig_AssetExtension,
            EntityInstance_AssetExtension,
        };

        static const std::array<std::function<AssetTypes(const Path& filename)>,
                                2>
            asset_loader = {
                LoadAssetFromPath<InputConfig>,
                LoadAssetFromPath<EntityInstance>,
            };

        static const std::array<std::function<AssetTypes()>, 2> asset_creator =
            {
                CreateAsset<InputConfig>,
                CreateAsset<EntityInstance>,
            };
        
        if (ImGui::BeginPopupModal("popup")) {
            for (size_t i = 0; i < asset_types.size(); i++) {
                auto asset_type = asset_types[i];
                if (ImGui::Selectable(asset_type, false)) {
                    m_asset_index = i;
                    if (m_mode == Mode::Open) {
                        FileDialog file_dialog{FileDialog::Type::OpenFile};
                        file_dialog.SetTitle("Open Asset");
                        file_dialog.AddFilter(asset_type,
                                              asset_extensions[i].data());
                        file_dialog.SetDefaultFolder(
                            std::filesystem::current_path().string());
                        file_dialog.Open();

                        auto& files = file_dialog.GetSelectedFiles();
                        if (files.empty()) {
                            break;
                        }

                        m_filename = files[0];
                        m_asset = asset_loader[i](m_filename);
                    } else if (m_mode == Mode::Create) {
                        m_asset = asset_creator[i]();
                    }
                    m_mode = Mode::None;
                }
            }
            ImGui::EndPopup();
        }

        if (ImGui::Button("Open")) {
            ImGui::OpenPopup("popup");
            m_mode = Mode::Open;
        }
        if (ImGui::Button("Create")) {
            ImGui::OpenPopup("popup");
            m_mode = Mode::Create;
        }
        if (!std::holds_alternative<std::monostate>(m_asset)) {
            if (ImGui::Button("Save")) {
                if (m_filename.empty()) {
                    FileDialog file_dialog{FileDialog::Type::SaveFile};
                    file_dialog.SetTitle("Save New Asset");
                    file_dialog.AddFilter(
                        asset_types[m_asset_index.value()],
                        asset_extensions[m_asset_index.value()].data());
                    file_dialog.Open();
                    file_dialog.SetDefaultFolder(
                        std::filesystem::current_path().string());

                    auto& filenames = file_dialog.GetSelectedFiles();
                    if (!filenames.empty()) {
                        auto saver = AssetSaver{m_filename};
                        std::visit(saver, m_asset);
                    }
                } else {
                    auto saver = AssetSaver{m_filename};
                    std::visit(saver, m_asset);
                }
            }
        }

        std::visit(AssetDisplay{}, m_asset);
    }

    ImGui::End();
}