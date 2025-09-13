#pragma once
#include "asset_viewer.hpp"
#include "engine/context.hpp"
#include "inspector.hpp"

namespace editor {
class EditorContext : public CommonContext {
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

    void HandleEvents(const SDL_Event&) override;

    void Update() override;
    const Path& GetProjectPath() const;
    bool IsRunningGame() const;

private:
    static std::unique_ptr<EditorContext> instance;
    Path m_project_path;
    FPSOption m_fps_option = FPSOption::FPS_60;
    std::unique_ptr<Inspector> m_editor_inspector;
    std::unique_ptr<Inspector> m_game_inspector;
    AssetViewer m_asset_viewer;

    EditorContext();
    void parseProjectPath();
    void controlFPS(TimeType elapse_time);
    void handleCamera();
    void showMainMenu();
};
}  // namespace editor

#define EDITOR_CONTEXT editor::EditorContext::GetInst()
