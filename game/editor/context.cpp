#include "context.hpp"

#include "engine/asset_manager.hpp"
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

EditorContext::~EditorContext() {}

void EditorContext::Initialize() {
    CommonContext::Initialize();

    m_window->SetTitle("treasure looter editor");
    m_window->Resize({1920, 1080});

    parseProjectPath();
}

void EditorContext::Update() {
    m_renderer->Clear();
    beginImGui();

    m_editor.Update();
    m_inspector.Update();

    endImGui();
    m_renderer->Present();
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

#ifdef TL_ENABLE_EDITOR
    m_project_path = node->value();
#endif
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

}
