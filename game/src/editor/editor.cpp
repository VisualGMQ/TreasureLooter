#include "editor/editor.hpp"

#include "animation.hpp"
#include "asset_manager.hpp"
#include "dialog.hpp"
#include "imgui.h"
#include "level.hpp"
#include "relationship.hpp"
#include "schema/display/config.hpp"
#include "schema/display/input.hpp"
#include "schema/display/level_content.hpp"
#include "schema/display/prefab.hpp"
#include "schema/serialize/asset_extensions.hpp"
#include "schema/serialize/config.hpp"
#include "schema/serialize/input.hpp"
#include "schema/serialize/level_content.hpp"
#include "schema/serialize/prefab.hpp"
#include "sdl_call.hpp"
#include "sprite.hpp"

#include <array>

struct AssetDisplay {
    template <typename T>
    void operator()(AssetLoadResult<T>& payload) {
        InstanceDisplay("Asset", payload.m_payload);
    }

    void operator()(std::monostate) const {}
};

struct AssetSyncHelper {
    template <typename T>
    void operator()(AssetLoadResult<T>& payload) {
        GAME_CONTEXT.m_assets_manager->GetManager<T>().Reload(payload.m_uuid);
    }

    void operator()(AssetLoadResult<InputConfig>& payload) {
        auto handle =
            GAME_CONTEXT.m_assets_manager->GetManager<InputConfig>().Replace(
                payload.m_uuid, payload.m_payload);
        GAME_CONTEXT.m_input_manager->SetConfig(GAME_CONTEXT, handle);
    }

    void operator()(AssetLoadResult<Prefab>& payload) {
        GAME_CONTEXT.m_assets_manager->GetManager<Prefab>().Replace(
            payload.m_uuid, payload.m_payload);
        GAME_CONTEXT.m_level_manager->GetCurrentLevel()
            ->ReloadEntitiesFromPrefab(payload.m_uuid);
    }

    void operator()(std::monostate) {}
};

void AddEntityToScene(Entity entity, AssetLoadResult<Prefab>& instance) {
    auto level = GAME_CONTEXT.m_level_manager->GetCurrentLevel();
    auto root_entity = level->GetRootEntity();
    auto relationship = GAME_CONTEXT.m_relationship_manager->Get(root_entity);
    relationship->m_children.push_back(entity);

    AssetSyncHelper helper;
    helper(instance);
}

template <typename T>
AssetTypes LoadAssetFromPath(const Path& filename) {
    auto result = LoadAsset<T>(filename);
    return AssetTypes{std::move(result)};
}

template <>
AssetTypes LoadAssetFromPath<Prefab>(const Path& filename) {
    auto result = LoadAsset<Prefab>(filename);

    AddEntityToScene(GAME_CONTEXT.CreateEntity(), result);
    return AssetTypes{result};
}

struct AssetSaver {
    AssetSaver(const Path& filename) : m_filename(filename) {}

    template <typename T>
    void operator()(AssetLoadResult<T>& payload) {
        if (!payload.m_uuid) {
            payload.m_uuid = UUID::CreateV4();
        }
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
    return AssetTypes{std::move(result)};
}

void Editor::Update() {
    if (ImGui::Begin("Editor", &m_open)) {
        static const std::array<const char*, 5> asset_types = {
            "InputConfig", "EntityInstance", "Animation", "GameConfig", "Level",
        };

        static const std::array<std::string_view, 5> asset_extensions = {
            InputConfig_AssetExtension,  Prefab_AssetExtension,
            Animation_AssetExtension,    GameConfig_AssetExtension,
            LevelContent_AssetExtension,
        };

        static const std::array<std::function<AssetTypes(const Path& filename)>,
                                5>
            asset_loader = {
                LoadAssetFromPath<InputConfig>,  LoadAssetFromPath<Prefab>,
                LoadAssetFromPath<Animation>,    LoadAssetFromPath<GameConfig>,
                LoadAssetFromPath<LevelContent>,
            };

        static const std::array<std::function<AssetTypes()>, 5> asset_creator =
            {
                CreateAsset<InputConfig>,  CreateAsset<Prefab>,
                CreateAsset<Animation>,    CreateAsset<GameConfig>,
                CreateAsset<LevelContent>,
            };

        if (ImGui::BeginPopupModal("popup")) {
            for (size_t i = 0; i < asset_types.size(); i++) {
                auto asset_type = asset_types[i];
                if (ImGui::Selectable(asset_type, false)) {
                    m_asset_index = i;
                    if (m_mode == Mode::Create) {
                        m_asset = asset_creator[i]();
                    }
                    m_mode = Mode::None;
                }
            }
            ImGui::EndPopup();
        }

        if (ImGui::Button("Open")) {
            FileDialog file_dialog{FileDialog::Type::OpenFile};
            file_dialog.SetTitle("Open Asset");
            file_dialog.SetDefaultFolder(
                std::filesystem::current_path().string());
            file_dialog.Open();

            auto& files = file_dialog.GetSelectedFiles();
            if (!files.empty()) {
                m_filename = files[0];
                auto file_str = m_filename.string();
                auto extension = file_str.substr(file_str.find('.'));

                auto it = std::find(asset_extensions.begin(),
                                    asset_extensions.end(), extension);
                if (it == asset_extensions.end()) {
                    SDL_CALL(SDL_ShowSimpleMessageBox(
                        SDL_MESSAGEBOX_WARNING, "Warning", "unknown file type",
                        nullptr));
                    LOGW("no registered asset type: {}", extension);
                } else {
                    m_asset =
                        asset_loader[it - asset_extensions.begin()](m_filename);
                    m_mode = Mode::Open;
                }
            }
        }

        ImGui::PushID(ImGuiIDGenerator::Gen());
        if (ImGui::Button("Create")) {
            ImGui::PopID();
            ImGui::OpenPopup("popup");
            m_mode = Mode::Create;
        } else {
            ImGui::PopID();
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
                        m_filename = filenames[0];
                        m_filename.replace_extension(
                            asset_extensions[m_asset_index.value()].data());
                        auto saver = AssetSaver{m_filename};
                        std::visit(saver, m_asset);
                    }
                } else {
                    auto saver = AssetSaver{m_filename};
                    std::visit(saver, m_asset);
                }
            }
            if (ImGui::Button("Sync")) {
                std::visit(AssetSyncHelper{}, m_asset);
            }
        }

        std::visit(AssetDisplay{}, m_asset);
    }

    ImGui::End();
}