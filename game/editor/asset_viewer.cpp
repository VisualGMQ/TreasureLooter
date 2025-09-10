#include "asset_viewer.hpp"
#include "engine/asset_manager.hpp"
#include "engine/dialog.hpp"
#include "engine/imgui_id_generator.hpp"
#include "imgui.h"
#include "schema/display/display.hpp"

AssetViewer::AssetViewer() {
    m_id = ImGuiIDGenerator::Gen();
    changeID({});
}

void AssetViewer::LoadAsset(const Path& filename) {
    m_asset = LoadVariantAsset(filename);
    m_current_open_filename = filename;
}

Path appendExtension(const Path& path, const std::string& extension) {
    std::string filename = path.string();
    if (path.has_extension()) {
        auto ext = filename.substr(filename.find_last_of("."));
        if (extension != ext) {
            filename += extension;
        }
    } else {
        filename += extension;
    }

    return filename;
}

namespace internal {
struct AssetFilterGetter {
    template <typename... Args>
    std::vector<Filter> operator()(TypeList<Args...>) {
        std::vector<Filter> filters;
        (filters.push_back(Filter{AssetInfoManager::GetName<Args>(),
                                  AssetInfoManager::GetExtension<Args>()}),
         ...);
        return filters;
    }
};

struct AssetCreator {
    template <typename... Args>
    Path operator()(TypeList<Args...>) {
        Path path;
        (tryCreate<Args>(path), ...);
        return path;
    }

private:
    Path m_filename;

    template <typename T>
    void tryCreate(Path& out_path) {
        auto name = AssetInfoManager::GetName<T>();
        auto extension = AssetInfoManager::GetExtension<T>();
        std::string label =
            std::string{name} + "(" + std::string{extension} + ")";
        if (ImGui::MenuItem(label.c_str())) {
            FileDialog file_dialog{FileDialog::Type::SaveFile};
            file_dialog.SetTitle("New Asset");
            Filter filter;
            filter.name = name;
            filter.pattern = extension;
            file_dialog.AddFilter(filter);
            file_dialog.Open();
            file_dialog.SetDefaultFolder(
                std::filesystem::current_path().string());

            auto& filenames = file_dialog.GetSelectedFiles();
            if (!filenames.empty()) {
                out_path = appendExtension(filenames[0], filter.pattern);
                EDITOR_CONTEXT.m_assets_manager->GetManager<T>().Create(
                    {}, out_path);
            }
        }
    }
};

}  // namespace internal

std::vector<Filter> GetAssetFilters() {
    internal::AssetFilterGetter getter;
    return getter(TypeList{});
}

Filter GetAssetFilterByType(const VariantAsset& asset) {
    return std::visit(
        [](const auto& payload) {
            using type = std::decay_t<decltype(payload)>;
            if constexpr (std::is_same_v<type, std::monostate>) {
                return Filter{};
            } else {
                using payload_type = typename type::underlying_type;
                auto extension = AssetInfoManager::GetExtension<payload_type>();
                auto name = AssetInfoManager::GetName<payload_type>();
                return Filter{name.data(), extension.data()};
            }
        },
        asset);
}

void AssetViewer::Update() {
    ImGui::SetNextWindowSize({m_window_size.w, m_window_size.h});
    if (m_is_open &&
        ImGui::Begin(("AssetViewer" + m_current_open_filename.string()).c_str(),
                     &m_is_open, ImGuiWindowFlags_MenuBar)) {
        auto new_size = ImGui::GetWindowSize();
        m_window_size.w = new_size.x;
        m_window_size.h = new_size.y;

        showMenu();

        if (!std::holds_alternative<std::monostate>(m_asset)) {
            std::visit(
                [](auto& payload) {
                    using type = std::decay_t<decltype(payload)>;
                    if constexpr (!std::is_same_v<type, std::monostate>) {
                        InstanceDisplay(
                            AssetInfoManager::GetName<type>().data(),
                            *payload.Get());
                    }
                },
                m_asset);
        }

        ImGui::End();
    }
}

void AssetViewer::changeID(const Path& filename) {
    if (filename.empty()) {
        window_id = "AssetViewer##" + std::to_string(m_id);
    } else {
        window_id =
            "AssetViewer - " + filename.string() + "##" + std::to_string(m_id);
    }
}

void AssetViewer::showMenu() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu("New")) {
                internal::AssetCreator creator;
                m_current_open_filename = creator(AssetTypeList{});
                if (!m_current_open_filename.empty()) {
                    LoadAsset(m_current_open_filename);
                }

                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Open", nullptr)) {
                FileDialog file_dialog{FileDialog::Type::OpenFile};
                file_dialog.SetTitle("Open Asset");
                auto filters = GetAssetFilters();
                for (auto& filter : filters) {
                    file_dialog.AddFilter(filter);
                }
                file_dialog.Open();
                file_dialog.SetDefaultFolder(
                    std::filesystem::current_path().string());

                auto& filenames = file_dialog.GetSelectedFiles();
                if (!filenames.empty()) {
                    this->LoadAsset(filenames[0]);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save", nullptr)) {
                SaveVariantAsset(m_asset);
            }
            if (ImGui::MenuItem("Save as", nullptr)) {
                FileDialog file_dialog{FileDialog::Type::SaveFile};
                file_dialog.SetTitle("Save as");
                auto filter = GetAssetFilterByType(m_asset);
                file_dialog.AddFilter(filter);
                file_dialog.Open();
                file_dialog.SetDefaultFolder(
                    std::filesystem::current_path().string());

                auto& filenames = file_dialog.GetSelectedFiles();
                if (!filenames.empty()) {
                    auto filename =
                        appendExtension(filenames[0], filter.pattern);
                    SaveAsVariantAsset(m_asset, filename);

                    this->LoadAsset(filename);
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}
