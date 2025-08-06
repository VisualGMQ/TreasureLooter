#include "instance_display.hpp"

#include "asset_manager.hpp"
#include "dialog.hpp"
#include "image.hpp"
#include "imgui.h"
#include "imgui_id_generator.hpp"
#include "math.hpp"

void InstanceDisplay(const char* name, int& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragInt(name, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, char& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_S8, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, short& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_S16, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, long long& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_S64, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const int& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::DragInt(name, (int*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const char& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_S8, (void*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const short& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_S16, (void*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const long long& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_S64, (void*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned int& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_U32, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned char& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_U8, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned short& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_U16, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned long long& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_U64, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned const int& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_U32, (unsigned int*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned const char& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_U8, (void*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned const short& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_U16, (void*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned const long long& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_U64, (void*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, bool& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::Checkbox(name, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const bool& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::Checkbox(name, (bool*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, float& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragFloat(name, &value, 0.1);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const float& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::DragFloat(name, (float*)&value, 01);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, double& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_Double, &value, 0.1);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const double& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_Double, (void*)&value, 0.1);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, std::string& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    char buf[1024] = {0};
    memcpy(buf, value.data(), value.size());
    ImGui::InputText(name, buf, sizeof(buf) - 1);
    value = buf;

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const std::string& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::InputText(name, (char*)value.data(), value.size());
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, std::string_view value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::InputText(name, (char*)value.data(), value.size());
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, Vec2& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragFloat2(name, (float*)&value, 0.1);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Vec2& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::DragFloat2(name, (float*)&value, 0.1);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, Region& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::Text("%s", name);
    InstanceDisplay("top left", value.m_topleft);
    InstanceDisplay("size", value.m_size);

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Region& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    InstanceDisplay("top left", value.m_topleft);
    InstanceDisplay("size", value.m_size);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, Degrees& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    float degree = value.Value();
    ImGui::DragFloat(name, &degree, 0.1);
    value = degree;

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Degrees& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    float degree = value.Value();
    ImGui::DragFloat(name, &degree, 0.1);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, Radians& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    float degree = Degrees{value}.Value();
    ImGui::DragFloat(name, &degree, 0.1);
    value = Degrees{degree};

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Radians& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    float degree = Degrees{value}.Value();
    ImGui::DragFloat(name, &degree, 0.1);
    ImGui::EndDisabled();

    ImGui::PopID();
}

template <typename T>
void showAssetSelectFile(Handle<T>& value, const std::vector<Filter>& filters) {
#ifdef TL_ENABLE_EDITOR
    std::string button_text = "none";

    if (value && value.GetFilename()) {
        button_text = value.GetFilename()->string();
    }

    ImGui::PushID(ImGuiIDGenerator::Gen());
    if (ImGui::Button(button_text.c_str())) {
        FileDialog dialog{FileDialog::Type::OpenFile};
        auto base_path = GAME_CONTEXT.GetProjectPath();
        dialog.SetTitle("Select Image");
        for (auto& filter : filters) {
            dialog.AddFilter(filter);
        }
        dialog.SetDefaultFolder(base_path);
        dialog.Open();

        auto& files = dialog.GetSelectedFiles();
        if (!files.empty()) {
            auto& filename = files[0];

            std::error_code err;
            auto relative_path =
                std::filesystem::relative(filename, base_path, err);
            std::string relative_path_str = relative_path.string();
            std::replace_if(
                relative_path_str.begin(), relative_path_str.end(),
                [](char c) { return c == '\\'; }, '/');
            relative_path = relative_path_str;

            if (err) {
                LOGE("Can only select file under {} dir", base_path);
            } else {
                value = GAME_CONTEXT.m_assets_manager->GetManager<T>().Load(
                    relative_path);
            }
        }
    }
    ImGui::PopID();
#endif
}

template <typename T>
void displayAssetName(Handle<T> handle) {
    std::string text = "none";

    if (handle && handle.GetFilename()) {
        text = handle.GetFilename()->string();
    }
    ImGui::BeginDisabled(true);

    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::InputText("filename", text.data(), text.size());
    ImGui::PopID();
}

void InstanceDisplay(const char* name, Handle<Image>& value) {
    ImGui::Text("%s", name);

    showAssetSelectFile(
        value, {
                   {   "Png", "png"},
                   {  "JPEG", "jpg"},
                   {"Bitmap", "bmp"}
    });

    if (value) {
        ImVec2 size;
        size.x = value->GetSize().x;
        size.y = value->GetSize().y;
        ImGui::Image(value->GetTexture(), size);
    }
}

void InstanceDisplay(const char* name, const Handle<Image>& value) {
    ImGui::Text("%s", name);
    displayAssetName(value);

    if (value) {
        ImVec2 size;
        size.x = value->GetSize().x;
        size.y = value->GetSize().y;
        ImGui::Image(value->GetTexture(), size);
    }

    ImGui::EndDisabled();
}

void InstanceDisplay(const char* name, Transform& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::Text("%s", name);
    InstanceDisplay("position", value.m_position);
    InstanceDisplay("scale", value.m_scale);
    InstanceDisplay("rotation", value.m_rotation);

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Transform& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    InstanceDisplay("position", value.m_position);
    InstanceDisplay("scale", value.m_scale);
    InstanceDisplay("rotation", value.m_rotation);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, TilemapHandle& value) {
    ImGui::Text("%s", name);
    showAssetSelectFile(value, {
                                   {"TileMap", "tmx"}
    });
}

void InstanceDisplay(const char* name, const TilemapHandle& value) {
    ImGui::Text("%s", name);
    displayAssetName(value);
}

#define HANDLE_TRACK_DISPLAY(binding) if (binding_point == binding)
#define HANDLE_LINEAR_TRACK_DISPLAY()                                  \
    if (track_base.GetType() == AnimationTrackType::Linear) {          \
        auto& track = static_cast<                                     \
            AnimationTrack<TARGET_TYPE, AnimationTrackType::Linear>&>( \
            track_base);                                               \
        InstanceDisplay("track", track);                               \
    }
#define HANDLE_DISCRETE_TRACK_DISPLAY()                                  \
    if (track_base.GetType() == AnimationTrackType::Discrete) {          \
        auto& track = static_cast<                                       \
            AnimationTrack<TARGET_TYPE, AnimationTrackType::Discrete>&>( \
            track_base);                                                 \
        InstanceDisplay("track", track);                                 \
    }

void animTrackDisplay(AnimationBindingPoint binding_point,
                      AnimationTrackBase& track_base) {
#define TARGET_TYPE Vec2
    HANDLE_TRACK_DISPLAY(AnimationBindingPoint::TransformPosition) {
        HANDLE_LINEAR_TRACK_DISPLAY();
        HANDLE_DISCRETE_TRACK_DISPLAY();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_TRACK_DISPLAY(AnimationBindingPoint::TransformScale) {
        HANDLE_LINEAR_TRACK_DISPLAY();
        HANDLE_DISCRETE_TRACK_DISPLAY();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Degrees
    HANDLE_TRACK_DISPLAY(AnimationBindingPoint::TransformRotation) {
        HANDLE_LINEAR_TRACK_DISPLAY();
        HANDLE_DISCRETE_TRACK_DISPLAY();
    }
#undef TARGET_TYPE

#define TARGET_TYPE ImageHandle
    HANDLE_TRACK_DISPLAY(AnimationBindingPoint::SpriteImage) {
        HANDLE_DISCRETE_TRACK_DISPLAY();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Region
    HANDLE_TRACK_DISPLAY(AnimationBindingPoint::SpriteRegion) {
        HANDLE_DISCRETE_TRACK_DISPLAY();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Flags<Flip>
    HANDLE_TRACK_DISPLAY(AnimationBindingPoint::SpriteFlip) {
        HANDLE_DISCRETE_TRACK_DISPLAY();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_TRACK_DISPLAY(AnimationBindingPoint::SpriteSize) {
        HANDLE_DISCRETE_TRACK_DISPLAY();
        HANDLE_LINEAR_TRACK_DISPLAY();
    }
#undef TARGET_TYPE
}

#undef HANDLE_TRACK_DISPLAY
#undef HANDLE_LINEAR_TRACK_DISPLAY
#undef HANDLE_DISCRETE_TRACK_DISPLAY

#define HANDLE_ANIM_DISPLAY(binding) if (binding_point == binding)
#define HANDLE_ANIM_LINEAR_DISPLAY()                                    \
    if (track_type == AnimationTrackType::Linear) {                     \
        tracks[binding_point] = std::make_unique<                       \
            AnimationTrack<TARGET_TYPE, AnimationTrackType::Linear>>(); \
    }
#define HANDLE_ANIM_DISCRETE_DISPLAY()                                    \
    if (track_type == AnimationTrackType::Discrete) {                     \
        tracks[binding_point] = std::make_unique<                         \
            AnimationTrack<TARGET_TYPE, AnimationTrackType::Discrete>>(); \
    }

void displayAnimationContent(Animation& anim) {
    auto& tracks = anim.GetTracks();

    ImGui::PushID(ImGuiIDGenerator::Gen());
    if (ImGui::Button("Create")) {
        ImGui::PopID();
        ImGui::OpenPopup("create new track");
    } else {
        ImGui::PopID();
    }

    for (auto& [binding, track] : tracks) {
        ImGui::PushID(ImGuiIDGenerator::Gen());
        if (ImGui::TreeNode("track")) {
            if (ImGui::Button("delete")) {
                tracks.erase(binding);
                ImGui::TreePop();
                ImGui::PopID();
                break;
            }
            InstanceDisplay("binding point", binding);
            animTrackDisplay(binding, *track);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    // popup window
    static AnimationBindingPoint binding_point = AnimationBindingPoint::Unknown;
    static AnimationTrackType track_type = AnimationTrackType::Linear;

    if (ImGui::BeginPopup("create new track")) {
        InstanceDisplay("binding point", binding_point);
        InstanceDisplay("track type", track_type);
        if (ImGui::Button("Create")) {
#define TARGET_TYPE Vec2
            HANDLE_ANIM_DISPLAY(AnimationBindingPoint::TransformPosition) {
                HANDLE_ANIM_LINEAR_DISPLAY();
                HANDLE_ANIM_DISCRETE_DISPLAY()
            }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
            HANDLE_ANIM_DISPLAY(AnimationBindingPoint::TransformScale) {
                HANDLE_ANIM_LINEAR_DISPLAY();
                HANDLE_ANIM_DISCRETE_DISPLAY()
            }
#undef TARGET_TYPE

#define TARGET_TYPE Degrees
            HANDLE_ANIM_DISPLAY(AnimationBindingPoint::TransformRotation) {
                HANDLE_ANIM_LINEAR_DISPLAY();
                HANDLE_ANIM_DISCRETE_DISPLAY();
            }
#undef TARGET_TYPE

#define TARGET_TYPE ImageHandle
            HANDLE_ANIM_DISPLAY(AnimationBindingPoint::SpriteImage) {
                HANDLE_ANIM_DISCRETE_DISPLAY();
            }
#undef TARGET_TYPE

#define TARGET_TYPE Region
            HANDLE_ANIM_DISPLAY(AnimationBindingPoint::SpriteRegion) {
                HANDLE_ANIM_DISCRETE_DISPLAY();
            }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
            HANDLE_ANIM_DISPLAY(AnimationBindingPoint::SpriteSize) {
                HANDLE_ANIM_LINEAR_DISPLAY();
                HANDLE_ANIM_DISCRETE_DISPLAY();
            }
#undef TARGET_TYPE

#define TARGET_TYPE Flags<Flip>
            HANDLE_ANIM_DISPLAY(AnimationBindingPoint::SpriteFlip) {
                HANDLE_ANIM_DISCRETE_DISPLAY();
            }
#undef TARGET_TYPE
        }
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void InstanceDisplay(const char* name, Handle<Animation>& animation) {
    ImGui::Text("%s", name);
    showAssetSelectFile(
        animation,
        {
            {"Animation", Animation_AssetExtension.substr(1).data()}
    });

    if (animation) {
        displayAnimationContent(*animation);
    }
}

void InstanceDisplay(const char* name, const Handle<Animation>& anim) {
    ImGui::Text("%s", name);
    displayAssetName(anim);

    if (anim) {
        // NOTE: dangerous operation!
        displayAnimationContent((Animation&)*anim);
    }
}

void InstanceDisplay(const char* name, Animation& animation) {
    ImGui::Text("%s", name);
    displayAnimationContent(animation);
}

void InstanceDisplay(const char* name, const Animation& anim) {
    ImGui::Text("%s", name);

    // NOTE: dangerous operation!
    displayAnimationContent((Animation&)anim);
}

void InstanceDisplay(const char* name, AnimationPlayer& player) {
    ImGui::Text("%s", name);
    auto animation = player.GetAnimation();
    if (!animation) {
        ImGui::PushID(ImGuiIDGenerator::Gen());
        constexpr std::string_view text = "no animation";
        ImGui::BeginDisabled(true);
        ImGui::InputText("animation", (char*)text.data(), text.size());
        ImGui::EndDisabled();
        ImGui::PopID();
        return;
    }

    int loop = player.GetLoopCount();
    InstanceDisplay("loop", loop);

    float rate = player.GetRate();
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragFloat("rate", &rate, 0.1, 0, FLT_MAX);
    ImGui::PopID();
    player.SetRate(rate);

    if (ImGui::Button("play")) {
        player.Play();
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
        player.Pause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        player.Stop();
    }
    ImGui::SameLine();
    if (ImGui::Button("Rewind")) {
        player.Rewind();
    }

    ImGui::Text("state: %lf/%lf", player.GetCurTime(), player.GetMaxTime());

    AnimationHandle old_handle = animation;
    InstanceDisplay("animation", animation);
    if (old_handle != animation) {
        player.ChangeAnimation(animation);
    }
}

void InstanceDisplay(const char* name, Entity e) {
    ImGui::Text("%s", name);
    ImGui::Text("entity: %uld", static_cast<uint32_t>(e));
}

void InstanceDisplay(const char* name, NullEntity) {
    ImGui::Text("%s", name);
    ImGui::Text("entity: null_entity");
}

void InstanceDisplay(const char* name, const Path& path) {
    auto string = path.string();
    ImGui::BeginDisabled(true);
    ImGui::InputText(name, string.data(), string.size());
    ImGui::EndDisabled();
}

void InstanceDisplay(const char* name, Path& path) {
    ImGui::BeginDisabled(true);
    auto string = path.string();
    ImGui::InputText(name, string.data(), string.size());
    if (ImGui::IsItemClicked()) {
        FileDialog file_dialog{FileDialog::Type::OpenFile};
        file_dialog.SetDefaultFolder(std::filesystem::current_path());
        file_dialog.Open();
        auto& files = file_dialog.GetSelectedFiles();
        if (!files.empty()) {
            path = files.front();
        }
    }
    ImGui::EndDisabled();
}