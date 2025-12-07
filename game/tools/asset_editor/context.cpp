#include "context.hpp"
#include "engine/asset_manager.hpp"
#include "engine/dialog.hpp"
#include "engine/relationship.hpp"
#include "engine/storage.hpp"
#include "imgui.h"
#include "schema/display/display.hpp"
#include "variant_asset.hpp"

std::unique_ptr<AssetEditorContext> AssetEditorContext::instance;

void AssetEditorContext::Init() {
    if (!instance) {
        instance =
            std::unique_ptr<AssetEditorContext>(new AssetEditorContext());
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

void AssetEditorContext::Initialize() {
    CommonContext::Initialize();

    m_window->SetTitle("TreasureLooter AssetEditor");
    m_window->Resize({720, 680});

    parseProjectPath();
}

void AssetEditorContext::Shutdown() {
    CommonContext::Shutdown();
}

void AssetEditorContext::parseProjectPath() {
    auto file =
        IOStream::CreateFromFile("project_path.xml", IOMode::Read, true);
    if (!file) {
        return;
    }

    auto content = file->Read();
    content.push_back('\0');

    rapidxml::xml_document<> doc;
    doc.parse<0>(content.data());
    auto node = doc.first_node("ProjectPath");
    if (!node) {
        LOGE("no ProjectPath node in project_path.xml");
        return;
    }

    m_project_path = node->value();
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

void AssetEditorContext::Update() {
    m_renderer->Clear();
    beginImGui();

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

    endImGui();
    m_renderer->Present();
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
        auto filename = filenames[0].string() + filter.pattern;
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
    m_asset = LoadVariantAsset(filename);
    changeAssetPathInTitle(*GetAssetFilename(m_asset));
}

void AssetEditorContext::HandleEvents(const SDL_Event& event) {
    CommonContext::HandleEvents(event);
}

void AssetEditorContext::changeAssetPathInTitle(const Path& path) {
    if (path.empty()) {
        m_window->SetTitle("TreasureLooter AssetEditor - [No Name]");
    } else {
        m_window->SetTitle("TreasureLooter AssetEditor - " + path.string());
    }
}
