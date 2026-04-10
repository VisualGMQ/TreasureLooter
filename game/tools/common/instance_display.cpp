#include "instance_display.hpp"

#include "engine/asset_manager.hpp"
#include "engine/cct.hpp"
#include "engine/dialog.hpp"
#include "engine/image.hpp"
#include "engine/math.hpp"
#include "imgui.h"
#include "schema/display/anim.hpp"
#include "schema/display/bind_point_schema.hpp"
#include "schema/display/collision_group_schema.hpp"
#include "schema/display/common.hpp"

void InstanceDisplay(const char* name, int& value) {
    ImGui::PushID(&value);
    ImGui::DragInt(name, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, char& value) {
    ImGui::PushID(&value);
    ImGui::DragScalar(name, ImGuiDataType_S8, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, short& value) {
    ImGui::PushID(&value);
    ImGui::DragScalar(name, ImGuiDataType_S16, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, long long& value) {
    ImGui::PushID(&value);
    ImGui::DragScalar(name, ImGuiDataType_S64, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const int& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::DragInt(name, (int*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const char& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_S8, (void*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const short& value) {
    ImGui::PushID(value);

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_S16, (void*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const long long& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_S64, (void*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned int& value) {
    ImGui::PushID(&value);
    ImGui::DragScalar(name, ImGuiDataType_U32, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned char& value) {
    ImGui::PushID(&value);
    ImGui::DragScalar(name, ImGuiDataType_U8, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned short& value) {
    ImGui::PushID(&value);
    ImGui::DragScalar(name, ImGuiDataType_U16, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned long long& value) {
    ImGui::PushID(&value);
    ImGui::DragScalar(name, ImGuiDataType_U64, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned const int& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_U32, (unsigned int*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned const char& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_U8, (void*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned const short& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_U16, (void*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned const long long& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_U64, (void*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, bool& value) {
    ImGui::PushID(&value);
    ImGui::Checkbox(name, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const bool& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::Checkbox(name, (bool*)&value);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, float& value) {
    ImGui::PushID(&value);
    ImGui::DragFloat(name, &value, 0.1);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const float& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::DragFloat(name, (float*)&value, 01);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, double& value) {
    ImGui::PushID(&value);
    ImGui::DragScalar(name, ImGuiDataType_Double, &value, 0.1);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const double& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_Double, (void*)&value, 0.1);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, std::string& value) {
    ImGui::PushID(&value);

    char buf[1024] = {0};
    memcpy(buf, value.data(), value.size());
    ImGui::InputText(name, buf, sizeof(buf) - 1);
    value = buf;

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const std::string& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::InputText(name, (char*)value.data(), value.size());
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, std::string_view value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::InputText(name, (char*)value.data(), value.size());
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Vec2& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::DragFloat2(name, (float*)&value, 0.1);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, Region& value) {
    ImGui::PushID(&value);

    ImGui::Text("%s", name);
    InstanceDisplay("top left", value.m_topleft);
    InstanceDisplay("size", value.m_size);

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Region& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    InstanceDisplay("top left", value.m_topleft);
    InstanceDisplay("size", value.m_size);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, Degrees& value) {
    ImGui::PushID(&value);

    float degree = value.Value();
    ImGui::DragFloat(name, &degree, 0.1);
    value = degree;

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Degrees& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    float degree = value.Value();
    ImGui::DragFloat(name, &degree, 0.1);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, Radians& value) {
    ImGui::PushID(&value);

    float degree = Degrees{value}.Value();
    ImGui::DragFloat(name, &degree, 0.1);
    value = Degrees{degree};

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Radians& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    float degree = Degrees{value}.Value();
    ImGui::DragFloat(name, &degree, 0.1);
    ImGui::EndDisabled();

    ImGui::PopID();
}

template <typename T>
void displayAssetName(Handle<T> handle) {
    std::string text = "none";

    if (handle && handle.GetFilename()) {
        text = handle.GetFilename()->string();
    }
    ImGui::BeginDisabled(true);

    ImGui::PushID(handle.Get());
    ImGui::InputText("filename", text.data(), text.size());
    ImGui::PopID();

    ImGui::EndDisabled();
}

void InstanceDisplay(const char* name, Handle<Image>& value) {
    HandleInstanceDisplayCommon(
        name, value,
        {
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
}

void InstanceDisplay(const char* name, Handle<Font>& value) {
    HandleInstanceDisplayCommon(name, value,
                                {
                                    {"TTF", "ttf"}
    });
}

void InstanceDisplay(const char* name, const Handle<Font>& value) {
    ImGui::Text("%s", name);
    displayAssetName(value);
}

void InstanceDisplay(const char* name, Transform& value) {
    ImGui::PushID(&value);

    ImGui::Text("%s", name);
    InstanceDisplay("position", value.m_position);
    InstanceDisplay("scale", value.m_scale);
    InstanceDisplay("rotation", value.m_rotation);

    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Transform& value) {
    ImGui::PushID(&value);

    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    InstanceDisplay("position", value.m_position);
    InstanceDisplay("scale", value.m_scale);
    InstanceDisplay("rotation", value.m_rotation);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, TilemapHandle& value) {
    HandleInstanceDisplayCommon(name, value,
                                {
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

#define TARGET_TYPE Vec2
    HANDLE_TRACK_DISPLAY(AnimationBindingPoint::SpriteRegionPosition) {
        HANDLE_DISCRETE_TRACK_DISPLAY();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_TRACK_DISPLAY(AnimationBindingPoint::SpriteRegionSize) {
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

#define TARGET_TYPE Vec2
    HANDLE_TRACK_DISPLAY(AnimationBindingPoint::SpriteAnchor) {
        HANDLE_DISCRETE_TRACK_DISPLAY();
        HANDLE_LINEAR_TRACK_DISPLAY();
    }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
    HANDLE_TRACK_DISPLAY(AnimationBindingPoint::BindPoint) {
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

    if (ImGui::Button("create ordinary animation")) {
        ImGui::OpenPopup("create new track");
    } else if (ImGui::Button("create sprite animation row column animation")) {
        ImGui::OpenPopup("create sprite region row column animation");
    } else if (ImGui::Button("create bind point track")) {
        ImGui::OpenPopup("create bind point track");
    }

    for (auto& [binding, track] : tracks) {
        ImGui::PushID(&track);
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

    auto& bind_point_tracks = anim.GetBindPointTracks();
    if (!bind_point_tracks.empty()) {
        ImGui::SeparatorText("bind point tracks");
        for (auto& [name, track] : bind_point_tracks) {
            ImGui::PushID(&track);
            if (ImGui::TreeNode("track")) {
                if (ImGui::Button("delete")) {
                    bind_point_tracks.erase(name);
                    ImGui::TreePop();
                    ImGui::PopID();
                    break;
                }
                InstanceDisplay("name", name);
                animTrackDisplay(AnimationBindingPoint::BindPoint, *track);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
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

#define TARGET_TYPE Vec2
            HANDLE_ANIM_DISPLAY(AnimationBindingPoint::SpriteRegionPosition) {
                HANDLE_ANIM_DISCRETE_DISPLAY();
            }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
            HANDLE_ANIM_DISPLAY(AnimationBindingPoint::SpriteRegionSize) {
                HANDLE_ANIM_DISCRETE_DISPLAY();
            }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
            HANDLE_ANIM_DISPLAY(AnimationBindingPoint::SpriteSize) {
                HANDLE_ANIM_LINEAR_DISPLAY();
                HANDLE_ANIM_DISCRETE_DISPLAY();
            }
#undef TARGET_TYPE

#define TARGET_TYPE Vec2
            HANDLE_ANIM_DISPLAY(AnimationBindingPoint::SpriteAnchor) {
                HANDLE_ANIM_LINEAR_DISPLAY();
                HANDLE_ANIM_DISCRETE_DISPLAY();
            }
#undef TARGET_TYPE

#define TARGET_TYPE Flags<Flip>
            HANDLE_ANIM_DISPLAY(AnimationBindingPoint::SpriteFlip) {
                HANDLE_ANIM_DISCRETE_DISPLAY();
            }
#undef TARGET_TYPE
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("create sprite region row column animation")) {
        static SpriteRowColumnAnimationInfo info;
        InstanceDisplay("info", info);

        if (ImGui::Button("Create")) {
            anim.AddTracks(info);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("create bind point track")) {
        static std::string name;
        InstanceDisplay("name", name);

        if (ImGui::Button("Create")) {
            std::unique_ptr<IAnimationTrack<Vec2>> track;
            if (track_type == AnimationTrackType::Linear) {
                track = std::make_unique<
                    AnimationTrack<Vec2, AnimationTrackType::Linear>>();
            } else if (track_type == AnimationTrackType::Discrete) {
                track = std::make_unique<
                    AnimationTrack<Vec2, AnimationTrackType::Discrete>>();
            }
            anim.AddBindPointTrack(name, std::move(track));
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void InstanceDisplay(const char* name, Handle<Animation>& animation) {
    HandleInstanceDisplayCommon(
        name, animation,
        {
            {"Animation", std::string{Animation_AssetExtension.substr(1)}}
    });

    if (animation) {
        displayAnimationContent(*animation);
    }
}

void InstanceDisplay(const char* name, const Handle<Animation>& anim) {
    ImGui::Text("%s", name);
    displayAssetName(anim);

    if (anim) {
        // NOTE: cast const to non-const is dangerous operation!
        displayAnimationContent((Animation&)*anim);
    }
}

void InstanceDisplay(const char* name, Animation& animation) {
    ImGui::Text("%s", name);
    displayAnimationContent(animation);
}

void InstanceDisplay(const char* name, const Animation& anim) {
    ImGui::Text("%s", name);

    // NOTE: cast const to non-const is dangerous operation!
    displayAnimationContent((Animation&)anim);
}

void InstanceDisplay(const char* name, AnimationPlayer& player) {
    ImGui::Text("%s", name);

    auto animation = player.GetAnimation();
    int loop = player.GetLoopCount();
    InstanceDisplay("loop", loop);

    float rate = player.GetRate();
    ImGui::PushID(&player);
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

    bool auto_play = player.IsAutoPlayEnabled();
    ImGui::Checkbox("auto play", &auto_play);
    player.EnableAutoPlay(auto_play);

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
    ImGui::InputText(name, string.data(), string.size(),
                     ImGuiInputTextFlags_ReadOnly);
    ImGui::EndDisabled();
}

void InstanceDisplay(const char* name, Path& path) {
    static char buf[1024] = {0};
    auto string = path.string();
    strcpy(buf, string.data());
    ImGui::InputText(name, buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Change")) {
        FileDialog file_dialog{FileDialog::Type::OpenFile};
        file_dialog.SetDefaultFolder(std::filesystem::current_path());
        file_dialog.Open();
        auto& files = file_dialog.GetSelectedFiles();
        if (!files.empty()) {
            path = files.front();
            const Path& project_path = CURRENT_CONTEXT.GetProjectPath();
            std::string str =
                std::filesystem::relative(path, project_path).string();
            std::replace(str.begin(), str.end(), '\\', '/');
            path = str;
        }
    }
}

void InstanceDisplay(const char* name, CharacterController& cct) {
    ImGui::Text("%s", name);

    ImGui::PushID(&cct);
    if (ImGui::TreeNode(name)) {
        InstanceDisplay("actor", *cct.GetActor());
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void InstanceDisplay(const char* name, CollisionGroup& group) {
    ImGui::Text("%s", name);
    ImGui::PushID(&group);

    std::vector<CollisionGroupType> collision_groups;
    for (int i = 0; i < sizeof(CollisionGroupType); i++) {
        auto type = static_cast<CollisionGroupType>(i);
        if (group.Has(type)) {
            collision_groups.push_back(type);
        }
    }

    static constexpr CollisionGroupType all_types[] = {
        CollisionGroupType::CCT,
        CollisionGroupType::Obstacle,
        CollisionGroupType::Coin,
        CollisionGroupType::WeaponAttack,
    };
    static constexpr const char* all_type_names[] = {
        "CCT",
        "Obstacle",
        "Coin",
        "WeaponAttack",
    };

    std::vector<size_t> available_type_indices;
    for (size_t i = 0; i < sizeof(all_types) / sizeof(all_types[0]); i++) {
        auto type = all_types[i];
        bool exists = false;
        for (auto existing : collision_groups) {
            if (existing == type) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            available_type_indices.push_back(i);
        }
    }

    static std::unordered_map<const void*, CollisionGroupType> add_type_cache;
    auto& add_type = add_type_cache[&group];
    if (!available_type_indices.empty()) {
        bool add_type_is_available = false;
        for (auto type_idx : available_type_indices) {
            auto available = all_types[type_idx];
            if (available == add_type) {
                add_type_is_available = true;
                break;
            }
        }
        if (!add_type_is_available) {
            add_type = all_types[available_type_indices.front()];
        }

        const char* preview = "Unknown";
        for (size_t i = 0; i < sizeof(all_types) / sizeof(all_types[0]); i++) {
            if (all_types[i] == add_type) {
                preview = all_type_names[i];
                break;
            }
        }
        if (ImGui::BeginCombo("add type", preview)) {
            for (auto type_idx : available_type_indices) {
                auto available = all_types[type_idx];
                const bool selected = (available == add_type);
                const char* label = all_type_names[type_idx];
                if (ImGui::Selectable(label, selected)) {
                    add_type = available;
                }
                if (selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("add")) {
            collision_groups.push_back(add_type);
        }
    } else {
        ImGui::BeginDisabled(true);
        ImGui::TextUnformatted("all collision group types have been added");
        ImGui::EndDisabled();
    }

    for (size_t i = 0; i < collision_groups.size(); i++) {
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::Button("del")) {
            collision_groups.erase(collision_groups.begin() + i);
            ImGui::PopID();
            break;
        }
        ImGui::SameLine();
        InstanceDisplay("group", collision_groups[i]);
        ImGui::PopID();
    }

    group.Clear();
    for (size_t i = 0; i < collision_groups.size(); i++) {
        bool duplicate = false;
        for (size_t j = 0; j < i; j++) {
            if (collision_groups[i] == collision_groups[j]) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) {
            continue;
        }
        auto type = collision_groups[i];
        group.Add(type);
    }
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const CollisionGroup& group) {
    ImGui::Text("%s", name);
    ImGui::PushID(&group);

    std::vector<CollisionGroupType> collision_groups;
    for (unsigned i = 0; i < sizeof(std::underlying_type_t<CollisionGroupType>);
         i++) {
        auto type = static_cast<CollisionGroupType>(i);
        if (group.Has(type)) {
            collision_groups.push_back(type);
        }
    }
    InstanceDisplay("groups", collision_groups);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Trigger& trigger) {
    ImGui::Text("%s", name);
    // TODO:
}

void InstanceDisplay(const char* name, PhysicsActor& actor) {
    ImGui::Text("%s", name);

    auto& shape = actor.GetShape();
    switch (shape.GetType()) {
        case PhysicsShapeType::Unknown:
            ImGui::Text("Unknown type");
            break;
        case PhysicsShapeType::Rect: {
            auto underlying = shape.AsRect();
            InstanceDisplay("rect", *underlying);
        } break;
        case PhysicsShapeType::Circle: {
            auto underlying = shape.AsCircle();
            InstanceDisplay("rect", *underlying);
        } break;
    }

    CollisionGroup mask = actor.GetCollisionMask();
    InstanceDisplay("collision mask", mask);
    actor.SetCollisionMask(mask);

    CollisionGroup layer = actor.GetCollisionLayer();
    InstanceDisplay("collision layer", layer);
    actor.SetCollisionLayer(layer);
}

void InstanceDisplay(const char* name, const Color& color) {
    ImGui::BeginDisabled(true);
    ImGui::DragFloat4(name, (float*)&color, 0.1, 0, 1);
    ImGui::EndDisabled();
}

void InstanceDisplay(const char* name, Color& color) {
    ImGui::DragFloat4(name, (float*)&color, 0.1, 0, 1);
}

void InstanceDisplay(const char* name, Image9Grid& grid) {
    ImGui::DragFloat("left", &grid.left);
    ImGui::DragFloat("right", &grid.right);
    ImGui::DragFloat("top", &grid.top);
    ImGui::DragFloat("bottom", &grid.bottom);
    ImGui::DragFloat("scale", &grid.scale);
}

void InstanceDisplay(const char* name, const Image9Grid& grid) {
    ImGui::BeginDisabled(true);
    ImGui::DragFloat("left", (float*)&grid.left);
    ImGui::DragFloat("right", (float*)&grid.right);
    ImGui::DragFloat("top", (float*)&grid.top);
    ImGui::DragFloat("bottom", (float*)&grid.bottom);
    ImGui::EndDisabled();
}
