#include "context.hpp"

#include "engine/asset_manager.hpp"
#include "engine/dialog.hpp"
#include "engine/relationship.hpp"
#include "engine/storage.hpp"
#include "imgui.h"
#include "imgui_internal.h"

namespace editor {
std::unique_ptr<EditorContext> EditorContext::instance;

void EditorContext::Init() {
    if (!instance) {
        instance = std::unique_ptr<EditorContext>(new EditorContext());
    } else {
        LOGW("inited context singleton twice!");
    }
}

void EditorContext::Destroy() {
    instance.reset();
}

EditorContext& EditorContext::GetInst() {
    return *instance;
}

EditorContext::~EditorContext() {
    GAME_CONTEXT.Destroy();
}

void EditorContext::Initialize() {
    CommonContext::Initialize();

    m_window->SetTitle("treasure looter editor");
    m_window->Resize({1920, 1080});

    parseProjectPath();

    auto path = Path{"game/editor/filename.prefab.xml"};
    auto str = path.string();

    m_editor_inspector = std::make_unique<Inspector>(EDITOR_CONTEXT);
    m_game_inspector = std::make_unique<Inspector>(GAME_CONTEXT);
    m_entity_prefab_component = std::make_unique<EntityPrefabComponent>();
}

void EditorContext::Shutdown() {
    m_entity_prefab_component.reset();

    CommonContext::Shutdown();
}

void EditorContext::HandleEvents(const SDL_Event& event) {
    CURRENT_CONTEXT.ChangeContext(EDITOR_CONTEXT);
    CommonContext::HandleEvents(event);

    CURRENT_CONTEXT.ChangeContext(GAME_CONTEXT);
    if (!GAME_CONTEXT.ShouldExit()) {
        GAME_CONTEXT.HandleEvents(event);
    }

    if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
        event.window.windowID == m_window->GetID()) {
        if (GAME_CONTEXT.IsInited()) {
            GAME_CONTEXT.Exit();
            GAME_CONTEXT.Shutdown();
        }
    }
    CURRENT_CONTEXT.ChangeContext(EDITOR_CONTEXT);
}

void EditorContext::Update() {
    // logic update
    if (GAME_CONTEXT.ShouldExit() && GAME_CONTEXT.IsInited()) {
        CURRENT_CONTEXT.ChangeContext(GAME_CONTEXT);
        GAME_CONTEXT.Shutdown();
        CURRENT_CONTEXT.ChangeContext(EDITOR_CONTEXT);
    }

    if (!GAME_CONTEXT.ShouldExit()) {
        CURRENT_CONTEXT.ChangeContext(GAME_CONTEXT);
        GAME_CONTEXT.Update();
        CURRENT_CONTEXT.ChangeContext(EDITOR_CONTEXT);
    }

    m_mouse->Update();
    m_keyboard->Update();

    handleCamera();
    m_relationship_manager->Update();

    // render update
    m_renderer->Clear();
    beginImGui();

    ImGui::ShowDemoWindow();
    showMainMenu();

    m_asset_viewer.Update();

    if (!GAME_CONTEXT.ShouldExit()) {
        m_game_inspector->Update();
    } else {
        m_editor_inspector->Update();
    }

    m_tilemap_component_manager->Update();
    m_sprite_manager->Update();
    m_renderer->DrawLine({-10000, 0}, {10000, 0}, Color::Red);
    m_renderer->DrawLine({0, -100000}, {0, 10000}, Color::Green);

    endImGui();

    m_renderer->Present();

    m_mouse->PostUpdate();
}

const Path& EditorContext::GetProjectPath() const {
    return m_project_path;
}

bool EditorContext::IsRunningGame() const {
    return GAME_CONTEXT.IsRunning();
}

EditorContext::EditorContext() {
    GAME_CONTEXT.Init();
}

void EditorContext::parseProjectPath() {
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

void EditorContext::controlFPS(TimeType elapse_time) {
    TimeType expect_one_frame_time = 0;
    switch (m_fps_option) {
        case FPSOption::FPS_15:
            expect_one_frame_time = 1.0f / 15.0f;
            break;
        case FPSOption::FPS_30:
            expect_one_frame_time = 1.0f / 30.0f;
            break;
        case FPSOption::FPS_60:
            expect_one_frame_time = 1.0f / 60.0f;
            break;
        case FPSOption::FPS_90:
            expect_one_frame_time = 1.0f / 90.0f;
            break;
        case FPSOption::FPS_120:
            expect_one_frame_time = 1.0f / 120.0f;
            break;
        case FPSOption::FPS_144:
            expect_one_frame_time = 1.0f / 144.0f;
            break;
        case FPSOption::NoLimit:
            expect_one_frame_time = 0;
            break;
    }

    if (expect_one_frame_time != 0 && elapse_time < expect_one_frame_time) {
        TimeType delay_time = expect_one_frame_time - elapse_time;
        SDL_Delay(delay_time * 1000);
    }
}

void EditorContext::handleCamera() {
    auto& mouse = EDITOR_CONTEXT.m_mouse;

    constexpr float scale_delta = 0.1;
    Vec2 scale = m_camera.GetScale();
    scale.x += mouse->Wheel() * scale_delta;
    scale.y += mouse->Wheel() * scale_delta;

    scale.x = Clamp(scale.x, 0.0001f, 1000.f);
    scale.y = Clamp(scale.x, 0.0001f, 1000.f);

    m_camera.ChangeScale(scale);

    if (mouse->Get(MouseButtonType::Right).IsPressing()) {
        m_camera.Move(-(mouse->Offset() / scale));
    }
}

void EditorContext::showMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Level")) {
            if (ImGui::MenuItem("New")) {
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

            if (ImGui::MenuItem("Open")) {
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

            ImGui::Separator();

            if (ImGui::MenuItem("Save")) {
                // TODO: not finish
            }
            if (ImGui::MenuItem("Save As")) {
                // TODO: not finish
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

        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::MenuItem(
                    "Show Physics Actor", 0,
                    EDITOR_CONTEXT.m_physics_scene->IsEnableDebugDraw())) {
                EDITOR_CONTEXT.m_physics_scene->ToggleDebugDraw();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Game")) {
            if (ImGui::MenuItem("Run")) {
                CURRENT_CONTEXT.ChangeContext(GAME_CONTEXT);
                GAME_CONTEXT.Initialize();
                ImGui::SetCurrentContext(m_imgui_context);
            }
            if (ImGui::MenuItem("Stop")) {
                GAME_CONTEXT.Exit();
                GAME_CONTEXT.Shutdown();
                CURRENT_CONTEXT.ChangeContext(EDITOR_CONTEXT);
                ImGui::SetCurrentContext(m_imgui_context);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

}  // namespace editor
