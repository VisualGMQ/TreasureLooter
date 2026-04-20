#include "context.hpp"
#include "client/animation_player.hpp"
#include "client/controller.hpp"
#include "client/draw_order.hpp"
#include "client/input/finger_touch.hpp"
#include "client/input/gamepad.hpp"
#include "client/input/input.hpp"
#include "client/input/keyboard.hpp"
#include "client/input/mouse.hpp"
#include "client/sprite.hpp"
#include "client/tilemap_render_component.hpp"
#include "common/asset_manager.hpp"
#include "common/bind_point.hpp"
#include "common/cct.hpp"
#include "common/debug_drawer.hpp"
#include "common/dialog.hpp"
#include "common/relationship.hpp"
#include "common/storage.hpp"
#include "common/trigger.hpp"
#include "imgui.h"
#include "instance_display.hpp"
#include "lyra/lyra.hpp"
#include "schema/display/display.hpp"
#include "variant_asset.hpp"

std::unique_ptr<AssetEditorContext> AssetEditorContext::instance;

void AssetEditorContext::Init() {
    if (!instance) {
        instance = std::unique_ptr<AssetEditorContext>(new AssetEditorContext);
    } else {
        LOGW("inited context singleton twice!");
    }
}

void AssetEditorContext::Destroy() {
    instance.reset();
}

AssetEditorContext& AssetEditorContext::GetInst() {
    return *instance;
}

void AssetEditorContext::Initialize(int argc, char** argv) {
    ToolContext::Initialize(argc, argv);

    m_window->SetTitle("TreasureLooter AssetEditor");
    m_window->Resize({720, 680});
    parseCmdArgs(argc, argv);
}

void AssetEditorContext::Shutdown() {
    ToolContext::Shutdown();
}

struct DisplayAsset {
    void operator()(std::monostate) {}

    template <typename T>
    void operator()(T handle) {
        using payload_type = typename T::underlying_type;
        InstanceDisplay(AssetInfoManager::GetName<payload_type>().data(),
                        *handle);
    }
};

void AssetEditorContext::update() {
    showMainMenu();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    if (ImGui::Begin("Fullscreen Window", nullptr, flags)) {
        std::visit(DisplayAsset{}, m_asset);
        ImGui::End();
    }
}

void AssetEditorContext::parseCmdArgs(int argc, char** argv) {
    std::filesystem::path filename;
    auto cli = lyra::cli() | lyra::opt(filename, "filename")["--filename"];
    lyra::parse_result result = cli.parse({argc, argv});

    if (!result) {
        LOGE("Command line parse failed: {}", result.message());
    }

    const bool is_regular = std::filesystem::is_regular_file(filename);
    if (is_regular) {
        auto path = filename;
        this->LoadAsset(path);
    } else if (!filename.empty()) {
        LOGW("asset editor cannot open file '{}', cwd='{}'", filename.string(),
             std::filesystem::current_path().string());
    }
}

void AssetEditorContext::showMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu("New")) {
                for (auto type : AssetInfoManager::GetNames()) {
                    if (ImGui::MenuItem(type.data())) {
                        m_asset = AssetInfoManager::CreateAsset(type, *this);
                        changeAssetPathInTitle({});
                    }
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
                if (!std::holds_alternative<std::monostate>(m_asset)) {
                    const Path* filename = GetAssetFilename(m_asset);
                    if (!filename) {
                        saveAs();
                    } else {
                        SaveVariantAsset(m_asset);
                        this->LoadAsset(*filename);
                    }
                }
            }
            if (ImGui::MenuItem("Save as", nullptr)) {
                saveAs();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void AssetEditorContext::saveAs() {
    FileDialog file_dialog{FileDialog::Type::SaveFile};
    file_dialog.SetTitle("Save as");
    auto filter = GetAssetFilterByType(m_asset);
    file_dialog.AddFilter(filter);
    file_dialog.Open();
    file_dialog.SetDefaultFolder(std::filesystem::current_path().string());

    auto& filenames = file_dialog.GetSelectedFiles();
    if (!filenames.empty()) {
        auto filename = filenames[0].string() +
                        std::string{GetAssetExtensionByType(m_asset)};
        SaveAsVariantAsset(m_asset, filename);
        this->LoadAsset(filename);
    }
}

struct AssetUnloadHelper {
public:
    void operator()(std::monostate) {}

    template <typename T>
    void operator()(T handle) {
        using payload_type = typename T::underlying_type;
        ASSET_VIEWER_CONTEXT.m_assets_manager->GetManager<payload_type>()
            .Unload(handle);
    }
};

void AssetEditorContext::LoadAsset(Path filename) {
    std::visit(AssetUnloadHelper{}, m_asset);
    m_asset = LoadVariantAsset(filename, *this);
    changeAssetPathInTitle(*GetAssetFilename(m_asset));
}

void AssetEditorContext::HandleEvents(const SDL_Event& event) {
    ToolContext::HandleEvents(event);
}

void AssetEditorContext::changeAssetPathInTitle(const Path& path) {
    if (path.empty()) {
        m_window->SetTitle("TreasureLooter AssetEditor - [No Name]");
    } else {
        m_window->SetTitle("TreasureLooter AssetEditor - " + path.string());
    }
}
