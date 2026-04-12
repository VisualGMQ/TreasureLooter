#include "context.hpp"

#include "engine/asset_manager.hpp"
#include "engine/bind_point.hpp"
#include "engine/dialog.hpp"
#include "engine/draw.hpp"
#include "engine/sprite.hpp"
#include "engine/transform.hpp"
#include "engine/input/input.hpp"
#include "engine/input/keyboard.hpp"
#include "engine/input/mouse.hpp"
#include "engine/input/finger_touch.hpp"
#include "engine/cct.hpp"
#include "engine/controller.hpp"
#include "engine/sprite.hpp"
#include "engine/debug_drawer.hpp"
#include "engine/trigger.hpp"
#include "engine/bind_point.hpp"
#include "imgui.h"
#include "instance_display.hpp"
#include "lyra/lyra.hpp"
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <filesystem>
#include <system_error>

std::unique_ptr<AnimationEditorContext> AnimationEditorContext::instance;

namespace {
constexpr float kTrackRowHeight = 28.0f;
constexpr float kTrackPadding = 10.0f;
constexpr float kTrackKeyRadius = 5.0f;
constexpr float kTrackTypeButtonWidth = 84.0f;

std::string_view trackName(AnimationBindingPoint binding) {
    switch (binding) {
        case AnimationBindingPoint::TransformPosition:
            return "Transform.Position";
        case AnimationBindingPoint::TransformScale:
            return "Transform.Scale";
        case AnimationBindingPoint::TransformRotation:
            return "Transform.Rotation";
        case AnimationBindingPoint::SpriteRegionPosition:
            return "Sprite.RegionPosition";
        case AnimationBindingPoint::SpriteRegionSize:
            return "Sprite.RegionSize";
        case AnimationBindingPoint::SpriteSize:
            return "Sprite.Size";
        case AnimationBindingPoint::SpriteImage:
            return "Sprite.Image";
        case AnimationBindingPoint::SpriteFlip:
            return "Sprite.Flip";
        case AnimationBindingPoint::SpriteAnchor:
            return "Sprite.Anchor";
        default:
            return "Unknown";
    }
}

bool supportLinear(AnimationBindingPoint binding) {
    return binding == AnimationBindingPoint::TransformPosition ||
           binding == AnimationBindingPoint::TransformScale ||
           binding == AnimationBindingPoint::TransformRotation ||
           binding == AnimationBindingPoint::SpriteSize ||
           binding == AnimationBindingPoint::SpriteAnchor ||
           binding == AnimationBindingPoint::BindPoint;
}

bool supportDiscrete(AnimationBindingPoint) {
    return true;
}

template <typename T>
std::vector<KeyFrame<T>>* tryGetKeyframes(AnimationTrackBase* track, AnimationTrackType type) {
    if (!track) {
        return nullptr;
    }
    if (type == AnimationTrackType::Linear) {
        auto* typed = dynamic_cast<AnimationTrack<T, AnimationTrackType::Linear>*>(track);
        return typed ? &typed->GetKeyframes() : nullptr;
    }
    auto* typed = dynamic_cast<AnimationTrack<T, AnimationTrackType::Discrete>*>(track);
    return typed ? &typed->GetKeyframes() : nullptr;
}

template <typename T>
void insertKeyframeByTime(std::vector<KeyFrame<T>>& keyframes, TimeType time, const T& value) {
    auto it = std::lower_bound(
        keyframes.begin(), keyframes.end(), time,
        [](const KeyFrame<T>& frame, TimeType t) { return frame.m_time < t; });
    keyframes.insert(it, KeyFrame<T>{value, time});
}

// Match a keyframe after reordering (sort); time compared with epsilon for double noise.
template <typename T>
bool keyframesMatchLogicalIdentity(const KeyFrame<T>& a, const KeyFrame<T>& b) {
    constexpr TimeType kEps = 1e-9;
    if (std::abs(a.m_time - b.m_time) > kEps) {
        return false;
    }
    return a.m_value == b.m_value;
}

template <typename T>
T defaultValueFromFrames(const std::vector<KeyFrame<T>>& keyframes) {
    if (keyframes.empty()) {
        return {};
    }
    return keyframes.back().m_value;
}

template <typename T>
std::unique_ptr<AnimationTrackBase> createTrackByType(AnimationTrackType type) {
    if (type == AnimationTrackType::Linear) {
        return std::make_unique<AnimationTrack<T, AnimationTrackType::Linear>>();
    }
    return std::make_unique<AnimationTrack<T, AnimationTrackType::Discrete>>();
}

template <typename T>
bool replaceTrackType(Animation& anim, bool is_bind_track, const std::string& bind_name,
                      AnimationBindingPoint binding, AnimationTrackType new_type,
                      const std::vector<KeyFrame<T>>& keyframes) {
    auto new_track = createTrackByType<T>(new_type);
    auto* target_frames = tryGetKeyframes<T>(new_track.get(), new_type);
    if (!target_frames) {
        return false;
    }
    for (auto& frame : keyframes) {
        target_frames->emplace_back(frame);
    }
    if (is_bind_track) {
        anim.GetBindPointTracks()[bind_name] = std::move(new_track);
    } else {
        anim.GetTracks()[binding] = std::move(new_track);
    }
    return true;
}

void drawDashedLine(ImDrawList* draw, float x0, float x1, float y, ImU32 color) {
    constexpr float dash = 6.0f;
    constexpr float gap = 4.0f;
    float x = x0;
    while (x < x1) {
        float end = std::min(x + dash, x1);
        draw->AddLine(ImVec2(x, y), ImVec2(end, y), color, 1.5f);
        x = end + gap;
    }
}
}  // namespace

void AnimationEditorContext::Init() {
    if (!instance) {
        instance = std::unique_ptr<AnimationEditorContext>(new AnimationEditorContext);
    } else {
        LOGW("inited context singleton twice!");
    }
}

void AnimationEditorContext::Destroy() {
    instance.reset();
}

AnimationEditorContext& AnimationEditorContext::GetInst() {
    return *instance;
}

void AnimationEditorContext::Initialize(int argc, char** argv) {
    ToolContext::Initialize(argc, argv);

    m_window->SetTitle("TreasureLooter AnimationEditor - [No Name]");
    m_window->Resize({1280, 800});
    parseCmdArgs(argc, argv);
}

void AnimationEditorContext::Shutdown() {
    clearPreviewEntity();
    ToolContext::Shutdown();
}

void AnimationEditorContext::HandleEvents(const SDL_Event& event) {
    ToolContext::HandleEvents(event);
}

void AnimationEditorContext::update() {
#ifdef IMGUI_HAS_DOCK
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                 ImGuiDockNodeFlags_PassthruCentralNode);
#endif
    showMainMenu();
    showInspectorPanel();
    showTimelinePanel();
    // After timeline scrub/seek, apply pose same frame before drawing the preview.
    renderScenePreview();

    if (m_preview_entity == null_entity || !m_should_play_preview) {
        return;
    }
    auto* player = m_animation_player_manager->Get(m_preview_entity);
    if (!player || !player->HasAnimation()) {
        return;
    }
    player->Update(ImGui::GetIO().DeltaTime);
    player->Sync(m_preview_entity);
}

void AnimationEditorContext::renderScenePreview() {
    if (m_preview_entity == null_entity) {
        return;
    }
    auto* sprite = m_sprite_manager->Get(m_preview_entity);
    if (!sprite || !sprite->m_image) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            Vec2 scale = m_camera.GetScale();
            scale.x = std::max(scale.x, 0.001f);
            scale.y = std::max(scale.y, 0.001f);
            m_camera.Move({-io.MouseDelta.x / scale.x, -io.MouseDelta.y / scale.y});
        }
        if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
            float factor = 1.0f + (-io.MouseDelta.y * 0.01f);
            factor = std::max(0.2f, factor);
            Vec2 scale = m_camera.GetScale() * factor;
            scale.x = Clamp(scale.x, 0.05f, 20.0f);
            scale.y = Clamp(scale.y, 0.05f, 20.0f);
            m_camera.ChangeScale(scale);
        }
        if (io.MouseWheel != 0.0f) {
            float factor = 1.0f + (io.MouseWheel * 0.1f);
            Vec2 scale = m_camera.GetScale() * factor;
            scale.x = Clamp(scale.x, 0.05f, 20.0f);
            scale.y = Clamp(scale.y, 0.05f, 20.0f);
            m_camera.ChangeScale(scale);
        }
    }

    // world origin helper axis for easy prefab positioning.
    constexpr float axis_len = 10000.0f;
    m_renderer->DrawLine({-axis_len, 0.0f}, {axis_len, 0.0f}, Color{1.0f, 0.2f, 0.2f, 0.8f}, -1000.0f, true);
    m_renderer->DrawLine({0.0f, -axis_len}, {0.0f, axis_len}, Color{0.2f, 1.0f, 0.2f, 0.8f}, -1000.0f, true);

    m_relationship_manager->Update();

    m_draw_order_manager->Update();

    DrawCommandSubmitter draw_cmd_submitter;
    draw_cmd_submitter.Submit();

    m_renderer->ApplyDrawcall();
}

void AnimationEditorContext::parseCmdArgs(int argc, char** argv) {
    std::filesystem::path filename;
    auto cli = lyra::cli() | lyra::opt(filename, "filename")["--filename"];
    lyra::parse_result result = cli.parse({argc, argv});
    if (!result) {
        LOGE("Command line parse failed: {}", result.message());
        return;
    }
    if (std::filesystem::is_regular_file(filename)) {
        loadAnimation(filename);
    }
}

void AnimationEditorContext::showMainMenu() {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Animation")) {
            newAnimation();
        }
        if (ImGui::MenuItem("Open Animation")) {
            FileDialog dialog{FileDialog::Type::OpenFile};
            dialog.SetTitle("Open Animation");
            dialog.AddFilter({"Animation XML", "animation.xml"});
            dialog.AddFilter({"Animation", "animation"});
            dialog.SetDefaultFolder(GetProjectPath());
            dialog.Open();
            auto& files = dialog.GetSelectedFiles();
            if (!files.empty()) {
                loadAnimation(files[0]);
            }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Save Animation")) {
            saveAnimation();
        }
        if (ImGui::MenuItem("Save Animation As")) {
            saveAnimationAs();
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

void AnimationEditorContext::showInspectorPanel() {
    if (!ImGui::Begin("Inspector")) {
        ImGui::End();
        return;
    }

    if (!m_current_animation) {
        ImGui::TextWrapped("Open an animation file (File → Open Animation) to begin editing.");
        ImGui::End();
        return;
    }

    if (ImGui::Button("Load Prefab for Preview")) {
        FileDialog dialog{FileDialog::Type::OpenFile};
        dialog.SetTitle("Load Prefab for Preview");
        dialog.AddFilter({"Prefab XML", "prefab.xml"});
        dialog.AddFilter({"Prefab", "prefab"});
        dialog.SetDefaultFolder(GetProjectPath());
        dialog.Open();
        auto& files = dialog.GetSelectedFiles();
        if (!files.empty()) {
            loadPrefab(files[0]);
        }
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(optional; does not modify the prefab file)");

    if (m_prefab) {
        if (auto filename = m_prefab.GetFilename(); filename) {
            std::string text = filename->string();
            ImGui::InputText("Preview prefab", text.data(), text.size() + 1, ImGuiInputTextFlags_ReadOnly);
        } else {
            ImGui::Text("Preview prefab: <embed>");
        }
    } else {
        ImGui::Text("Preview prefab: <none>");
    }

    auto* player = (m_preview_entity == null_entity) ? nullptr
                                                      : m_animation_player_manager->Get(m_preview_entity);
    if (!player) {
        ImGui::Separator();
        ImGui::TextWrapped("Internal error: no AnimationPlayer on preview entity.");
        ImGui::End();
        return;
    }

    ImGui::Separator();
    ImGui::Text("Time: %.3f / %.3f", player->GetCurTime(), player->GetMaxTime());
    ImGui::Text("Scale: %.2fx", m_camera.GetScale().x);

    ImGui::SeparatorText("Selected Keyframe");
    if (!m_selected_keyframe) {
        ImGui::TextUnformatted("No keyframe selected.");
        ImGui::End();
        return;
    }

    auto getSelectedTrack = [this]() -> AnimationTrackBase* {
        if (!m_current_animation || !m_selected_keyframe) {
            return nullptr;
        }
        auto& sel = *m_selected_keyframe;
        if (sel.m_is_bind_point_track) {
            auto& tracks = m_current_animation->GetBindPointTracks();
            auto it = tracks.find(sel.m_bind_point_name);
            return it == tracks.end() ? nullptr : it->second.get();
        }
        auto& tracks = m_current_animation->GetTracks();
        auto it = tracks.find(sel.m_binding);
        return it == tracks.end() ? nullptr : it->second.get();
    };

    AnimationTrackBase* selected_track = getSelectedTrack();
    if (!selected_track) {
        ImGui::TextUnformatted("Selected track no longer exists.");
        if (ImGui::Button("Clear Selection")) {
            clearSelectedKeyframe();
        }
        ImGui::End();
        return;
    }

    auto& sel = *m_selected_keyframe;
    std::string_view name = sel.m_is_bind_point_track ? sel.m_bind_point_name.c_str()
                                                  : trackName(sel.m_binding);
    ImGui::Text("Track: %s", name.data());
    ImGui::Text("Mode: %s", sel.m_track_type == AnimationTrackType::Linear ? "Linear" : "Discrete");

    bool removed = false;
    auto editFrame = [&](auto dummy) {
        using value_type = decltype(dummy);
        auto* frames = tryGetKeyframes<value_type>(selected_track, sel.m_track_type);
        if (!frames) {
            return;
        }
        if (sel.m_keyframe_index < 0 || sel.m_keyframe_index >= static_cast<int>(frames->size())) {
            return;
        }
        auto& frame = (*frames)[sel.m_keyframe_index];
        ImGui::DragScalar("Time", ImGuiDataType_Double, &frame.m_time, 0.01f, nullptr, nullptr);
        if (frame.m_time < 0.0) {
            frame.m_time = 0.0;
        }
        // Do not sort every frame: update() runs Inspector before the timeline, so sorting here
        // invalidated keyframe drag indices.
        const bool time_edit_finished = ImGui::IsItemDeactivatedAfterEdit();
        InstanceDisplay("Value", frame.m_value);
        if (ImGui::Button("Delete Keyframe")) {
            frames->erase(frames->begin() + sel.m_keyframe_index);
            removed = true;
        }
        if (time_edit_finished) {
            const KeyFrame<value_type> snapshot = frame;
            std::sort(frames->begin(), frames->end(), [](const auto& a, const auto& b) {
                return a.m_time < b.m_time;
            });
            for (int i = 0; i < static_cast<int>(frames->size()); ++i) {
                if (keyframesMatchLogicalIdentity((*frames)[i], snapshot)) {
                    sel.m_keyframe_index = i;
                    break;
                }
            }
        }
    };

    if (sel.m_is_bind_point_track || sel.m_binding == AnimationBindingPoint::TransformPosition ||
        sel.m_binding == AnimationBindingPoint::TransformScale ||
        sel.m_binding == AnimationBindingPoint::SpriteRegionPosition ||
        sel.m_binding == AnimationBindingPoint::SpriteRegionSize ||
        sel.m_binding == AnimationBindingPoint::SpriteSize ||
        sel.m_binding == AnimationBindingPoint::SpriteAnchor ||
        sel.m_binding == AnimationBindingPoint::BindPoint) {
        editFrame(Vec2{});
    } else if (sel.m_binding == AnimationBindingPoint::TransformRotation) {
        editFrame(Degrees{});
    } else if (sel.m_binding == AnimationBindingPoint::SpriteFlip) {
        editFrame(Flags<Flip>{});
    } else if (sel.m_binding == AnimationBindingPoint::SpriteImage) {
        editFrame(ImageHandle{});
    }

    if (removed) {
        clearSelectedKeyframe();
    }

    ImGui::End();
}

void AnimationEditorContext::showTimelinePanel() {
    if (!ImGui::Begin("Animation Timeline")) {
        ImGui::End();
        return;
    }
    if (!m_current_animation) {
        ImGui::TextWrapped("No animation selected. Use File → Open Animation.");
        ImGui::End();
        return;
    }
    auto* player = (m_preview_entity == null_entity) ? nullptr : m_animation_player_manager->Get(m_preview_entity);
    if (player) {
        if (ImGui::Button("Play")) {
            m_should_play_preview = true;
            player->Play();
        }
        ImGui::SameLine();
        if (ImGui::Button("Pause")) {
            m_should_play_preview = false;
            player->Pause();
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            m_should_play_preview = false;
            player->Stop();
        }
        ImGui::SameLine();
        if (ImGui::Button("Rewind")) {
            player->Rewind();
        }
        ImGui::SameLine();
        int loop = player->GetLoopCount();
        ImGui::SetNextItemWidth(90.0f);
        if (ImGui::DragInt("Loop", &loop, 1, -1, 999999)) {
            player->SetLoop(loop);
        }
        ImGui::SameLine();
        float rate = player->GetRate();
        ImGui::SetNextItemWidth(90.0f);
        if (ImGui::DragFloat("Rate", &rate, 0.05f, 0.0f, 100.0f)) {
            player->SetRate(rate);
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto Tick", &m_should_play_preview);
        ImGui::Separator();
    }

    TimeType max_time = 1.0f;
    for (auto& [_, track] : m_current_animation->GetTracks()) {
        max_time = std::max(max_time, track->GetFinishTime());
    }
    for (auto& [_, track] : m_current_animation->GetBindPointTracks()) {
        max_time = std::max(max_time, track->GetFinishTime());
    }

    ImGui::DragFloat("Zoom(px/s)", &m_timeline_zoom, 5.0f, 5.0f, 10000.0f);
    const float timeline_pixels =
        std::max<float>(50000.0f, max_time * m_timeline_zoom + 2000.0f);
    auto seekPreviewTo = [&](TimeType t) {
        if (!player || !player->HasAnimation()) {
            return;
        }
        const bool keep_playing = player->IsPlaying() && m_should_play_preview;
        const TimeType clamped_t =
            std::min<TimeType>(std::max<TimeType>(0.0, t), player->GetMaxTime());
        // Update() advances by (delta * rate). Here `clamped_t` is absolute animation time, not
        // clock delta — without forcing rate=1, scrubbing scales with Rate and can overshoot
        // GetMaxTime() in one step (e.g. jump to end when moving toward the second keyframe).
        const float saved_rate = player->GetRate();
        player->SetRate(1.0f);
        player->Rewind();
        player->Play();
        player->Update(clamped_t);
        player->Sync(m_preview_entity);
        player->SetRate(saved_rate);
        if (!keep_playing) {
            player->Pause();
        }
    };

    struct DragState {
        bool active = false;
        bool keyframe_awaiting_drag = false;
        bool is_bind = false;
        AnimationBindingPoint binding = AnimationBindingPoint::Unknown;
        std::string bind_name;
        AnimationTrackType type = AnimationTrackType::Discrete;
        int key_index = -1;
    };
    static DragState drag_state;
    static bool drag_playhead = false;

    auto tryDrawTrackTyped = [&](AnimationTrackBase* track_base, auto fn) {
        if (!track_base) {
            return false;
        }
        if (dynamic_cast<AnimationTrack<Vec2, AnimationTrackType::Linear>*>(track_base) ||
            dynamic_cast<AnimationTrack<Vec2, AnimationTrackType::Discrete>*>(track_base)) {
            fn(Vec2{});
            return true;
        }
        if (dynamic_cast<AnimationTrack<Degrees, AnimationTrackType::Linear>*>(track_base) ||
            dynamic_cast<AnimationTrack<Degrees, AnimationTrackType::Discrete>*>(track_base)) {
            fn(Degrees{});
            return true;
        }
        if (dynamic_cast<AnimationTrack<Flags<Flip>, AnimationTrackType::Discrete>*>(track_base)) {
            fn(Flags<Flip>{});
            return true;
        }
        if (dynamic_cast<AnimationTrack<ImageHandle, AnimationTrackType::Discrete>*>(track_base)) {
            fn(ImageHandle{});
            return true;
        }
        return false;
    };

    ImGui::BeginChild("timeline_scroll", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
    const ImVec2 child_origin = ImGui::GetCursorScreenPos();
    const float child_height = ImGui::GetContentRegionAvail().y + ImGui::GetStyle().ScrollbarSize;

    ImGui::SetCursorScreenPos(ImVec2(child_origin.x + m_track_name_panel_width - 2.0f, child_origin.y));
    ImGui::InvisibleButton("track_name_splitter", ImVec2(4.0f, std::max(60.0f, child_height)));
    if (ImGui::IsItemActive()) {
        m_track_name_panel_width += ImGui::GetIO().MouseDelta.x;
        m_track_name_panel_width = Clamp(m_track_name_panel_width, 180.0f, 900.0f);
    }
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(child_origin.x + m_track_name_panel_width, child_origin.y),
        ImVec2(child_origin.x + m_track_name_panel_width, child_origin.y + std::max(60.0f, child_height)),
        IM_COL32(100, 100, 100, 180), 1.0f);
    ImGui::SetCursorScreenPos(child_origin);

    const float axis_h = 28.0f;
    const float line_min_x = child_origin.x + m_track_name_panel_width + kTrackPadding;
    const float line_max_x = line_min_x + timeline_pixels - 2.0f * kTrackPadding - kTrackTypeButtonWidth;
    const float scroll_x = ImGui::GetScrollX();
    auto timeToRow = [&](TimeType time) -> float {
        return line_min_x - scroll_x + static_cast<float>(time * m_timeline_zoom);
    };
    auto rowToTime = [&](float x) -> TimeType {
        return std::max<TimeType>(0.0, static_cast<TimeType>((x - (line_min_x - scroll_x)) / m_timeline_zoom));
    };

    ImGui::InvisibleButton("timeline_axis_hit", ImVec2(m_track_name_panel_width + timeline_pixels, axis_h));
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 axis_min = child_origin;
    const ImVec2 axis_max{child_origin.x + m_track_name_panel_width + timeline_pixels, child_origin.y + axis_h};
    const bool axis_hovered = ImGui::IsMouseHoveringRect(axis_min, axis_max, true);
    draw->AddRectFilled(axis_min, axis_max, IM_COL32(20, 20, 20, 180));
    draw->AddLine(ImVec2(line_min_x, axis_max.y - 2.0f), ImVec2(line_max_x, axis_max.y - 2.0f), IM_COL32(180, 180, 180, 255), 1.2f);

    // Adaptive axis unit: dense zoom uses ms, far zoom switches to seconds.
    const float px_per_sec = m_timeline_zoom;
    float major_step_s = 1.0f;
    if (px_per_sec >= 4000.0f) major_step_s = 0.001f;
    else if (px_per_sec >= 2500.0f) major_step_s = 0.002f;
    else if (px_per_sec >= 1200.0f) major_step_s = 0.005f;
    else if (px_per_sec >= 700.0f) major_step_s = 0.01f;
    else if (px_per_sec >= 350.0f) major_step_s = 0.02f;
    else if (px_per_sec >= 180.0f) major_step_s = 0.05f;
    else if (px_per_sec >= 90.0f) major_step_s = 0.1f;
    else if (px_per_sec >= 45.0f) major_step_s = 0.2f;
    else if (px_per_sec >= 25.0f) major_step_s = 0.5f;
    else if (px_per_sec >= 12.0f) major_step_s = 1.0f;
    else if (px_per_sec >= 6.0f) major_step_s = 2.0f;
    else major_step_s = 5.0f;

    const float minor_step_s = major_step_s * 0.2f;
    const float max_s = timeline_pixels / m_timeline_zoom;
    float last_label_x = -1000.0f;
    for (float t = 0.0f; t < max_s; t += minor_step_s) {
        const float x = timeToRow(t);
        if (x > line_max_x) {
            break;
        }
        const float major_mod = std::fmod(t, major_step_s);
        const bool major = major_mod < 0.0001f || (major_step_s - major_mod) < 0.0001f;
        const float tick_h = major ? 11.0f : 6.0f;
        draw->AddLine(ImVec2(x, axis_max.y - 2.0f), ImVec2(x, axis_max.y - 2.0f - tick_h),
                      IM_COL32(200, 200, 200, major ? 255 : 140), 1.0f);
        if (major && (x - last_label_x) >= 70.0f) {
            char buf[32];
            if (major_step_s < 1.0f) {
                std::snprintf(buf, sizeof(buf), "%.0fms", t * 1000.0f);
            } else {
                std::snprintf(buf, sizeof(buf), "%.2fs", t);
            }
            draw->AddText(ImVec2(x + 2.0f, axis_min.y + 4.0f), IM_COL32(220, 220, 220, 255), buf);
            last_label_x = x;
        }
    }

    if (player && player->HasAnimation()) {
        const float playhead_x = timeToRow(player->GetCurTime());
        draw->AddLine(ImVec2(playhead_x, axis_min.y), ImVec2(playhead_x, child_origin.y + child_height),
                      IM_COL32(255, 80, 80, 220), 1.5f);

        if (axis_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            drag_playhead = true;
            seekPreviewTo(rowToTime(ImGui::GetIO().MousePos.x));
        }
        if (axis_hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !drag_playhead) {
            seekPreviewTo(rowToTime(ImGui::GetIO().MousePos.x));
        }
        if (drag_playhead && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            seekPreviewTo(rowToTime(ImGui::GetIO().MousePos.x));
        }
        if (drag_playhead && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            drag_playhead = false;
        }
    }

    struct PendingDeleteTrack {
        bool valid = false;
        bool is_bind = false;
        AnimationBindingPoint binding = AnimationBindingPoint::Unknown;
        std::string bind_name;
    } pending_delete;

    auto drawTrackRow = [&](const char* label, AnimationBindingPoint binding, bool is_bind_track,
                            const std::string& bind_name, AnimationTrackBase* track_base) {
        ImGui::PushID(track_base);
        const ImVec2 row_start = ImGui::GetCursorScreenPos();
        const float row_w = m_track_name_panel_width + timeline_pixels;

        ImGui::InvisibleButton("track_row_hit", ImVec2(row_w, kTrackRowHeight));
        const ImVec2 row_end{row_start.x + row_w, row_start.y + kTrackRowHeight};
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilled(row_start, row_end, IM_COL32(35, 35, 35, 80));

        const ImVec2 label_min = row_start;
        const ImVec2 label_max{row_start.x + m_track_name_panel_width, row_end.y};
        draw->AddRectFilled(label_min, label_max, IM_COL32(25, 25, 25, 120));
        draw->PushClipRect(label_min, label_max, true);
        draw->AddText(ImVec2(label_min.x + 8.0f, label_min.y + 6.0f), IM_COL32_WHITE, label);
        draw->PopClipRect();

        const float line_y = (row_start.y + row_end.y) * 0.5f;
        // Use the same time<->screen mapping as the ruler/playhead (line_min_x + scroll), not
        // row-local coords — tree indent + missing scroll_x made scrubbing far too sensitive.
        const float track_line_x0 = timeToRow(0);
        const float track_line_x1 = timeToRow(max_s);
        if (track_base->GetType() == AnimationTrackType::Discrete) {
            drawDashedLine(draw, track_line_x0, track_line_x1, line_y, IM_COL32(120, 120, 120, 255));
        } else {
            draw->AddLine(ImVec2(track_line_x0, line_y), ImVec2(track_line_x1, line_y),
                          IM_COL32(120, 120, 120, 255), 1.5f);
        }

        bool clicked_keyframe = false;
        auto drawAndHandleFrames = [&](auto dummy) {
            using value_type = decltype(dummy);
            auto* frames = tryGetKeyframes<value_type>(track_base, track_base->GetType());
            if (!frames) {
                return;
            }
            for (int i = 0; i < static_cast<int>(frames->size()); ++i) {
                const float x = timeToRow((*frames)[i].m_time);
                const bool selected = m_selected_keyframe &&
                                      m_selected_keyframe->m_is_bind_point_track == is_bind_track &&
                                      m_selected_keyframe->m_binding == binding &&
                                      m_selected_keyframe->m_bind_point_name == bind_name &&
                                      m_selected_keyframe->m_track_type == track_base->GetType() &&
                                      m_selected_keyframe->m_keyframe_index == i;
                const ImU32 color = selected ? IM_COL32(255, 210, 90, 255) : IM_COL32(240, 240, 240, 255);
                draw->AddCircleFilled(ImVec2(x, line_y), kTrackKeyRadius, color);

                const ImVec2 p = ImGui::GetIO().MousePos;
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
                    p.x >= x - kTrackKeyRadius && p.x <= x + kTrackKeyRadius &&
                    p.y >= line_y - kTrackKeyRadius && p.y <= line_y + kTrackKeyRadius) {
                    m_selected_keyframe = SelectedKeyframe{
                        is_bind_track, binding, bind_name, track_base->GetType(), i};
                    clicked_keyframe = true;
                    drag_state.active = false;
                    drag_state.keyframe_awaiting_drag = true;
                    drag_state.is_bind = is_bind_track;
                    drag_state.binding = binding;
                    drag_state.bind_name = bind_name;
                    drag_state.type = track_base->GetType();
                    drag_state.key_index = i;
                }
            }

            if (drag_state.keyframe_awaiting_drag &&
                drag_state.is_bind == is_bind_track && drag_state.binding == binding &&
                drag_state.bind_name == bind_name && drag_state.type == track_base->GetType() &&
                ImGui::IsMouseDragging(ImGuiMouseButton_Left, 5.0f)) {
                drag_state.active = true;
                drag_state.keyframe_awaiting_drag = false;
            }

            if (drag_state.active && ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
                drag_state.is_bind == is_bind_track && drag_state.binding == binding &&
                drag_state.bind_name == bind_name && drag_state.type == track_base->GetType() &&
                drag_state.key_index >= 0 && drag_state.key_index < static_cast<int>(frames->size())) {
                auto& frame = (*frames)[drag_state.key_index];
                frame.m_time = std::max<float>(0.0f, rowToTime(ImGui::GetIO().MousePos.x));
                seekPreviewTo(frame.m_time);
            }

            if (drag_state.active && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                if (drag_state.is_bind == is_bind_track && drag_state.binding == binding &&
                    drag_state.bind_name == bind_name && drag_state.type == track_base->GetType() &&
                    drag_state.key_index >= 0 && drag_state.key_index < static_cast<int>(frames->size())) {
                    const KeyFrame<value_type> moved = (*frames)[drag_state.key_index];
                    std::sort(frames->begin(), frames->end(), [](const auto& a, const auto& b) {
                        return a.m_time < b.m_time;
                    });
                    int new_index = drag_state.key_index;
                    bool found = false;
                    for (int idx = 0; idx < static_cast<int>(frames->size()); ++idx) {
                        if (keyframesMatchLogicalIdentity((*frames)[idx], moved)) {
                            new_index = idx;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        new_index = 0;
                    }
                    if (m_selected_keyframe) {
                        m_selected_keyframe->m_keyframe_index = new_index;
                    }
                    drag_state.key_index = new_index;
                    drag_state.active = false;
                }
            }

            const bool row_hovered = ImGui::IsItemHovered();
            const bool label_hovered = ImGui::IsMouseHoveringRect(label_min, label_max, true);
            if (label_hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                ImGui::OpenPopup("track_label_menu");
            } else if (row_hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                ImGui::OpenPopup("track_row_menu");
            }
            if (ImGui::BeginPopup("track_row_menu")) {
                const TimeType insert_time = rowToTime(ImGui::GetIO().MousePos.x);
                if (ImGui::MenuItem("Insert Keyframe Here")) {
                    insertKeyframeByTime(*frames, insert_time, defaultValueFromFrames(*frames));
                }
                ImGui::EndPopup();
            }
            if (ImGui::BeginPopup("track_label_menu")) {
                if (ImGui::MenuItem("Delete This Track")) {
                    pending_delete.valid = true;
                    pending_delete.is_bind = is_bind_track;
                    pending_delete.binding = binding;
                    pending_delete.bind_name = bind_name;
                }
                ImGui::EndPopup();
            }

            const bool can_to_linear =
                supportLinear(binding) && track_base->GetType() == AnimationTrackType::Discrete;
            const bool can_to_discrete =
                supportDiscrete(binding) && track_base->GetType() == AnimationTrackType::Linear;
            const bool can_toggle = can_to_linear || can_to_discrete;
            ImGui::SetCursorScreenPos(
                ImVec2(row_start.x + row_w - kTrackTypeButtonWidth - 6.0f, row_start.y + 4.0f));
            if (!can_toggle) {
                ImGui::BeginDisabled(true);
            }
            const char* preview = track_base->GetType() == AnimationTrackType::Linear ? "Linear" : "Discrete";
            if (ImGui::BeginCombo("##track_type", preview, ImGuiComboFlags_NoArrowButton) && can_toggle) {
                if (can_to_linear && ImGui::Selectable("Linear")) {
                    if (replaceTrackType<value_type>(
                            *m_current_animation, is_bind_track, bind_name, binding,
                            AnimationTrackType::Linear, *frames)) {
                        clearSelectedKeyframe();
                        refreshPlayerAnimationBinding();
                    }
                }
                if (can_to_discrete && ImGui::Selectable("Discrete")) {
                    if (replaceTrackType<value_type>(
                            *m_current_animation, is_bind_track, bind_name, binding,
                            AnimationTrackType::Discrete, *frames)) {
                        clearSelectedKeyframe();
                        refreshPlayerAnimationBinding();
                    }
                }
                ImGui::EndCombo();
            }
            if (!can_toggle) {
                ImGui::EndDisabled();
            }
        };
        tryDrawTrackTyped(track_base, drawAndHandleFrames);

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered() && !clicked_keyframe) {
            const TimeType clicked_time = std::max<float>(0.0f, rowToTime(ImGui::GetIO().MousePos.x));
            seekPreviewTo(clicked_time);
            clearSelectedKeyframe();
            drag_state.keyframe_awaiting_drag = false;
            drag_state.active = false;
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !clicked_keyframe &&
            !drag_state.active && !drag_state.keyframe_awaiting_drag) {
            const TimeType drag_time = std::max<float>(0.0f, rowToTime(ImGui::GetIO().MousePos.x));
            seekPreviewTo(drag_time);
        }
        ImGui::PopID();
    };

    if (ImGui::Button("+ Create Track")) {
        ImGui::OpenPopup("create_track_popup");
    }
    if (ImGui::BeginPopup("create_track_popup")) {
        static AnimationBindingPoint create_binding = AnimationBindingPoint::TransformPosition;
        static AnimationTrackType create_type = AnimationTrackType::Discrete;
        static std::string bind_name;
        InstanceDisplay("Property", create_binding);
        InstanceDisplay("Mode", create_type);
        if (create_binding == AnimationBindingPoint::BindPoint) {
            InstanceDisplay("Bind Name", bind_name);
        }
        if (ImGui::Button("Create")) {
            if (create_binding == AnimationBindingPoint::BindPoint) {
                if (!bind_name.empty()) {
                    m_current_animation->GetBindPointTracks()[bind_name] =
                        createTrackByType<Vec2>(create_type);
                }
            } else if (create_binding == AnimationBindingPoint::TransformRotation) {
                m_current_animation->GetTracks()[create_binding] =
                    createTrackByType<Degrees>(create_type);
            } else if (create_binding == AnimationBindingPoint::SpriteFlip) {
                m_current_animation->GetTracks()[create_binding] =
                    std::make_unique<AnimationTrack<Flags<Flip>, AnimationTrackType::Discrete>>();
            } else if (create_binding == AnimationBindingPoint::SpriteImage) {
                m_current_animation->GetTracks()[create_binding] =
                    std::make_unique<AnimationTrack<ImageHandle, AnimationTrackType::Discrete>>();
            } else {
                m_current_animation->GetTracks()[create_binding] =
                    createTrackByType<Vec2>(create_type);
            }
            refreshPlayerAnimationBinding();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::CollapsingHeader("Property Tracks", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (auto& [binding, track] : m_current_animation->GetTracks()) {
            std::string label = std::string(trackName(binding));
            drawTrackRow(label.c_str(), binding, false, "", track.get());
        }
    }

    if (ImGui::CollapsingHeader("Bind Point Tracks", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (auto& [name, track] : m_current_animation->GetBindPointTracks()) {
            std::string label = std::string("BindPoint.") + name;
            drawTrackRow(label.c_str(), AnimationBindingPoint::BindPoint, true, name, track.get());
        }
    }

    if (pending_delete.valid) {
        if (pending_delete.is_bind) {
            m_current_animation->GetBindPointTracks().erase(pending_delete.bind_name);
        } else {
            m_current_animation->GetTracks().erase(pending_delete.binding);
        }
        clearSelectedKeyframe();
        refreshPlayerAnimationBinding();
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        drag_state.keyframe_awaiting_drag = false;
        if (drag_state.active) {
            drag_state.active = false;
        }
    }

    ImGui::EndChild();

    ImGui::End();
}

void AnimationEditorContext::loadPrefab(Path filename) {
    auto relative = ToProjectRelative(filename);
    auto handle = m_assets_manager->GetManager<Prefab>().Load(relative, true);
    if (!handle) {
        LOGE("failed to load prefab '{}'", relative.string());
        return;
    }

    m_prefab = handle;
    clearPreviewEntity();
    m_preview_entity = CreateEntity();
    attachPreviewPrefab(*m_prefab);
    refreshPlayerAnimationBinding();
}

void AnimationEditorContext::clearPreviewEntity() {
    if (m_preview_entity == null_entity) {
        return;
    }
    m_sprite_manager->RemoveEntity(m_preview_entity);
    m_transform_manager->RemoveEntity(m_preview_entity);
    m_animation_player_manager->RemoveEntity(m_preview_entity);
    m_bind_point_component_manager->RemoveEntity(m_preview_entity);
    m_preview_entity = null_entity;
}

void AnimationEditorContext::attachPreviewPrefab(const Prefab& prefab) {
    if (m_preview_entity == null_entity) {
        return;
    }
    if (prefab.m_transform) {
        auto transform = prefab.m_transform.value();
        transform.m_position = {0, 0};
        m_transform_manager->ReplaceComponent(m_preview_entity, transform);
    } else {
        m_transform_manager->ReplaceComponent(m_preview_entity, Transform{});
    }
    if (prefab.m_sprite) {
        m_sprite_manager->ReplaceComponent(m_preview_entity, prefab.m_sprite.value());
    }
    if (prefab.m_animation) {
        m_animation_player_manager->RegisterEntity(m_preview_entity, prefab.m_animation.value());
    } else if (m_current_animation) {
        AnimationPlayerDefinition def;
        def.m_animation = m_current_animation;
        m_animation_player_manager->RegisterEntity(m_preview_entity, def);
    }
    if (!prefab.m_bind_points.empty()) {
        m_bind_point_component_manager->RegisterEntity(m_preview_entity, prefab.m_bind_points);
    }
}

void AnimationEditorContext::ensurePreviewPlayer() {
    if (!m_current_animation || m_preview_entity != null_entity) {
        return;
    }
    m_preview_entity = CreateEntity();
    m_transform_manager->ReplaceComponent(m_preview_entity, Transform{});
    Sprite sprite{};
    m_sprite_manager->ReplaceComponent(m_preview_entity, sprite);
    AnimationPlayerDefinition def;
    def.m_animation = m_current_animation;
    m_animation_player_manager->RegisterEntity(m_preview_entity, def);
}

void AnimationEditorContext::newAnimation() {
    m_current_animation = m_assets_manager->GetManager<Animation>().Create();
    m_animation_file_path.clear();
    clearSelectedKeyframe();
    changeWindowTitle({});
    refreshPlayerAnimationBinding();
}

void AnimationEditorContext::loadAnimation(Path filename) {
    auto relative = ToProjectRelative(filename);
    auto handle = m_assets_manager->GetManager<Animation>().Load(relative, true);
    if (!handle) {
        LOGE("failed to load animation '{}'", relative.string());
        return;
    }
    m_current_animation = handle;
    m_animation_file_path = relative;
    clearSelectedKeyframe();
    changeWindowTitle(relative);
    refreshPlayerAnimationBinding();
}

void AnimationEditorContext::saveAnimation() {
    if (!m_current_animation) {
        return;
    }
    if (m_animation_file_path.empty()) {
        saveAnimationAs();
        return;
    }
    SaveAsset(m_current_animation.GetUUID(), *m_current_animation, m_animation_file_path);
    loadAnimation(m_animation_file_path);
}

void AnimationEditorContext::saveAnimationAs() {
    if (!m_current_animation) {
        return;
    }
    FileDialog dialog{FileDialog::Type::SaveFile};
    dialog.SetTitle("Save Animation As");
    dialog.AddFilter({"Animation XML", "animation.xml"});
    dialog.SetDefaultFolder(GetProjectPath());
    dialog.Open();
    auto& files = dialog.GetSelectedFiles();
    if (files.empty()) {
        return;
    }
    Path final_file = files[0];
    auto extension = final_file.extension().string();
    if (extension != ".xml") {
        final_file += ".animation.xml";
    }
    auto relative = ToProjectRelative(final_file);
    SaveAsset(m_current_animation.GetUUID(), *m_current_animation, relative);
    loadAnimation(relative);
}

void AnimationEditorContext::refreshPlayerAnimationBinding() {
    if (m_current_animation && m_preview_entity == null_entity) {
        ensurePreviewPlayer();
    }
    if (m_preview_entity == null_entity) {
        return;
    }
    auto* player = m_animation_player_manager->Get(m_preview_entity);
    if (!player) {
        return;
    }
    if (m_current_animation) {
        player->ChangeAnimation(m_current_animation);
    } else if (player->HasAnimation()) {
        m_current_animation = player->GetAnimation();
        if (auto filename = m_current_animation.GetFilename(); filename) {
            m_animation_file_path = *filename;
            changeWindowTitle(m_animation_file_path);
        }
    }
}

void AnimationEditorContext::clearSelectedKeyframe() {
    m_selected_keyframe.reset();
}

void AnimationEditorContext::changeWindowTitle(const Path& path) {
    if (path.empty()) {
        m_window->SetTitle("TreasureLooter AnimationEditor - [No Name]");
    } else {
        m_window->SetTitle("TreasureLooter AnimationEditor - " + path.string());
    }
}
