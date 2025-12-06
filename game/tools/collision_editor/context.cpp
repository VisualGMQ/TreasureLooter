#include "context.hpp"

#include "engine/asset_manager.hpp"
#include "engine/dialog.hpp"
#include "engine/message_box.hpp"
#include "engine/relationship.hpp"
#include "engine/storage.hpp"
#include "imgui.h"
#include "schema/asset_info.hpp"
#include "schema/display/display.hpp"
#include "schema/serialize/level_content.hpp"
#include "schema/serialize/prefab.hpp"
#include "variant_asset.hpp"

std::unique_ptr<CollisionEditorContext> CollisionEditorContext::instance;

void CollisionEditorContext::Init() {
    if (!instance) {
        instance =
            std::unique_ptr<CollisionEditorContext>(new CollisionEditorContext);
    } else {
        LOGW("inited context singleton twice!");
    }
}

void CollisionEditorContext::Destroy() {
    instance.reset();
}

CollisionEditorContext& CollisionEditorContext::GetInst() {
    return *instance;
}

void CollisionEditorContext::Initialize() {
    ToolContext::Initialize();

    m_window->SetTitle("TreasureLooter CollisionEditor");
    m_window->Resize({1024, 720});

    m_level_manager->Load("assets/gpa/empty.level.xml");


    m_axis_gizmo = &m_gizmo_manager.CreateAxisGizmo();
    m_center_gizmo = &m_gizmo_manager.CreateButtonGizmo();
    m_rect_gizmo.m_up = &m_gizmo_manager.CreateButtonGizmo();
    m_rect_gizmo.m_down = &m_gizmo_manager.CreateButtonGizmo();
    m_rect_gizmo.m_left = &m_gizmo_manager.CreateButtonGizmo();
    m_rect_gizmo.m_right = &m_gizmo_manager.CreateButtonGizmo();
    m_radius_gizmo = &m_gizmo_manager.CreateButtonGizmo();
}

void CollisionEditorContext::Shutdown() {
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

void CollisionEditorContext::update() {
    m_sprite_manager->Update();

    m_gizmo_manager.Update(*this);
    m_gizmo_manager.Draw(*this);

    showMainMenu();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize({320, viewport->WorkSize.y});

    if (ImGui::Begin("Fullscreen Window", nullptr, flags)) {
        interactiveConfigPhysics();
        ImGui::End();
    }
}

void CollisionEditorContext::showMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", nullptr)) {
                FileDialog file_dialog{FileDialog::Type::OpenFile};
                file_dialog.SetTitle("Open Asset");
                Filter filter;
                filter.name = "Prefab";
                filter.pattern = Prefab_AssetExtension.substr(
                    Prefab_AssetExtension.find_first_of('.'));
                file_dialog.AddFilter(filter);
                file_dialog.SetDefaultFolder(
                    std::filesystem::current_path().string());
                auto& filenames = file_dialog.GetSelectedFiles();
                if (!filenames.empty()) {
                    LevelHandle level = m_level_manager->GetCurrentLevel();
                    m_prefab = m_assets_manager->GetManager<Prefab>().Load(
                        filenames[0]);
                    level->Instantiate(m_prefab);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save", nullptr)) {
                rapidxml::xml_document<> doc;
                SaveAsset(m_prefab.GetUUID(), *m_prefab.Get(), *m_prefab.GetFilename());
                MessageBox msgbox{"Save Prefab", "save prefab OK", MessageBox::Type::Info};
                msgbox.Show();
            }
            ImGui::EndMenu();
        }
        if (m_entity != null_entity) {
            if (ImGui::BeginMenu("Mode")) {
                if (ImGui::MenuItem("CCT", nullptr)) {
                    m_mode = Mode::CCT;
                    if (!m_cct_manager->Has(m_entity)) {
                        MessageBox msg_box{"CCT",
                                           "no CCT component, create one?",
                                           MessageBox::Type::Info};
                        auto yes_id = msg_box.AddReturnButton("Yes");
                        auto no_id = msg_box.AddReturnButton("No");
                        auto id = msg_box.Show();
                        if (id == yes_id) {
                            m_prefab->m_cct = {};
                            // TODO: init cct & gizmo
                        }
                    }
                }
                if (ImGui::MenuItem("trigger", nullptr)) {
                    m_mode = Mode::Trigger;
                    MessageBox msg_box{"Trigger",
                                       "no Trigger component, create one?",
                                       MessageBox::Type::Info};
                    auto yes_id = msg_box.AddReturnButton("Yes");
                    auto no_id = msg_box.AddReturnButton("No");
                    auto id = msg_box.Show();
                    if (id == yes_id) {
                        m_prefab->m_trigger = {};
                    }
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMainMenuBar();
    }
}

void CollisionEditorContext::HandleEvents(const SDL_Event& event) {
    ToolContext::HandleEvents(event);
}

void CollisionEditorContext::changeAssetPathInTitle(const Path& path) {
    if (path.empty()) {
        m_window->SetTitle("TreasureLooter CollisionEditor - [No Name]");
    } else {
        m_window->SetTitle("TreasureLooter CollisionEditor - " + path.string());
    }
}

void CollisionEditorContext::interactiveConfigPhysics() {
    PhysicsActor* actor = nullptr;

    if (m_mode == Mode::CCT) {
        CharacterController* cct = m_cct_manager->Get(m_entity);
        actor = cct->GetActor();
    } else if (m_mode == Mode::Trigger) {
        Trigger* trigger = m_trigger_component_manager->Get(m_entity);
        actor = trigger->GetActor();
    }

    if (actor == nullptr) {
        return;
    }

    // TODO: move actor using gizmo
}

void CollisionEditorContext::interactiveConfigCCT() {

}

void CollisionEditorContext::interactiveConfigTrigger() {

}
