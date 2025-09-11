#include "context.hpp"

#include "imgui.h"
#include "engine/asset_manager.hpp"
#include "engine/dialog.hpp"
#include "engine/relationship.hpp"
#include "engine/storage.hpp"

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
}

void EditorContext::Initialize() {
    CommonContext::Initialize();

    m_window->SetTitle("treasure looter editor");
    m_window->Resize({1920, 1080});

    parseProjectPath();

    auto path = Path{"game/editor/filename.prefab.xml"};
    auto str = path.string();
}

void EditorContext::Update() {
    m_mouse->Update();
    m_keyboard->Update();

    handleCamera();
    m_relationship_manager->Update();

    m_renderer->Clear();
    beginImGui();

    ImGui::ShowDemoWindow();

    m_asset_viewer.Update();
    m_inspector.Update();

    m_sprite_manager->Update();
    m_tilemap_component_manager->Update();

    endImGui();
    m_renderer->Present();

    m_mouse->PostUpdate();
}

const Path& EditorContext::GetProjectPath() const {
    return m_project_path;
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
    scale.x += mouse->GetWheel() * scale_delta;
    scale.y += mouse->GetWheel() * scale_delta;

    scale.x = Clamp(scale.x, 0.0001f, 1000.f);
    scale.y = Clamp(scale.x, 0.0001f, 1000.f);

    m_camera.ChangeScale(scale);

    if (mouse->Get(MouseButtonType::Right).IsPressing()) {
        m_camera.Move(-(mouse->GetOffset() / scale));
    }
}

}  // namespace editor
