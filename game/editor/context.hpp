#pragma once
#include "editor.hpp"
#include "engine/context.hpp"
#include "inspector.hpp"

namespace editor {
class EditorContext: public CommonContext {
public:
    static void Init();
    static void Destroy();
    static EditorContext& GetInst();

    EditorContext(const EditorContext&) = delete;
    EditorContext& operator=(const EditorContext&) = delete;
    EditorContext(EditorContext&&) = delete;
    EditorContext& operator=(EditorContext&&) = delete;
    ~EditorContext() override;

    void Initialize() override;

    void Update() override;
    const Path& GetProjectPath() const;

private:
    static std::unique_ptr<EditorContext> instance;
    Path m_project_path;
    FPSOption m_fps_option = FPSOption::FPS_60;

    EditorContext() = default;
    void parseProjectPath();
    void controlFPS(TimeType elapse_time);
    Inspector m_inspector;
    Editor m_editor;
};
}

#define EDITOR_CONTEXT editor::EditorContext::GetInst()
