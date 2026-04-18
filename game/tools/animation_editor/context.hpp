#pragma once

#include "engine/entity.hpp"
#include "engine/animation.hpp"
#include "engine/animation_player.hpp"
#include "schema/scene_definition.hpp"
#include "tool_context.hpp"
#include <memory>
#include <optional>
#include <string>

class AnimationEditorContext : public ToolContext {
public:
    static void Init();
    static void Destroy();
    static AnimationEditorContext& GetInst();

    void Initialize(int argc, char** argv) override;
    void Shutdown() override;
    void HandleEvents(const SDL_Event&) override;

protected:
    void update() override;

private:
    static std::unique_ptr<AnimationEditorContext> instance;

    PrefabHandle m_prefab;
    Entity m_preview_entity = null_entity;
    AnimationHandle m_current_animation;
    Path m_animation_file_path;
    bool m_should_play_preview = false;
    float m_timeline_zoom = 100.0f;
    float m_track_name_panel_width = 320.0f;

    struct SelectedKeyframe {
        bool m_is_bind_point_track = false;
        AnimationBindingPoint m_binding = AnimationBindingPoint::Unknown;
        std::string m_bind_point_name;
        AnimationTrackType m_track_type = AnimationTrackType::Discrete;
        int m_keyframe_index = -1;
    };
    std::optional<SelectedKeyframe> m_selected_keyframe;

    void parseCmdArgs(int argc, char** argv);
    void showMainMenu();
    void showInspectorPanel();
    void showTimelinePanel();
    void renderScenePreview();

    void loadPrefab(Path filename);
    void clearPreviewEntity();
    void attachPreviewPrefab(const Prefab& prefab);
    void ensurePreviewPlayer();

    void newAnimation();
    void loadAnimation(Path filename);
    void saveAnimation();
    void saveAnimationAs();

    void refreshPlayerAnimationBinding();
    void changeWindowTitle(const Path& path);
    void clearSelectedKeyframe();
};

#define ANIMATION_EDITOR_CONTEXT ::AnimationEditorContext::GetInst()
