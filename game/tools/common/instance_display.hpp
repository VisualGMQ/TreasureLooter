#pragma once
#include "engine/animation.hpp"
#include "engine/asset.hpp"
#include "engine/asset_manager.hpp"
#include "engine/context.hpp"
#include "engine/dialog.hpp"
#include "engine/entity.hpp"
#include "engine/handle.hpp"
#include "engine/math.hpp"
#include "engine/text.hpp"
#include "engine/trigger.hpp"
#include "engine/tilemap.hpp"
#include "schema/serialize/physics_schema.hpp"
#include "imgui.h"
#include "schema/asset_info.hpp"
#include "schema/display/display.hpp"
#include "schema/physics_schema.hpp"

#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

enum class Flip;
class Animation;
class AnimationPlayer;
struct Image9Grid;
class PhysicsShape;
class CollisionGroup;
class CharacterController;
class Trigger;
class Image;

void InstanceDisplay(const char* name, int& value);
void InstanceDisplay(const char* name, const int& value);
void InstanceDisplay(const char* name, char& value);
void InstanceDisplay(const char* name, const char& value);
void InstanceDisplay(const char* name, short& value);
void InstanceDisplay(const char* name, const short& value);
void InstanceDisplay(const char* name, long long& value);
void InstanceDisplay(const char* name, const long long& value);
void InstanceDisplay(const char* name, unsigned int& value);
void InstanceDisplay(const char* name, unsigned const int& value);
void InstanceDisplay(const char* name, unsigned char& value);
void InstanceDisplay(const char* name, unsigned const char& value);
void InstanceDisplay(const char* name, unsigned short& value);
void InstanceDisplay(const char* name, unsigned const short& value);
void InstanceDisplay(const char* name, unsigned long long& value);
void InstanceDisplay(const char* name, unsigned const long long& value);
void InstanceDisplay(const char* name, bool& value);
void InstanceDisplay(const char* name, const bool& value);
void InstanceDisplay(const char* name, float& value);
void InstanceDisplay(const char* name, const float& value);
void InstanceDisplay(const char* name, double& value);
void InstanceDisplay(const char* name, const double& value);
void InstanceDisplay(const char* name, std::string& value);
void InstanceDisplay(const char* name, const std::string& value);
void InstanceDisplay(const char* name, std::string_view value);
void InstanceDisplay(const char* name, Region& value);
void InstanceDisplay(const char* name, const Region& value);
void InstanceDisplay(const char* name, Degrees& value);
void InstanceDisplay(const char* name, const Degrees& value);
void InstanceDisplay(const char* name, Radians& value);
void InstanceDisplay(const char* name, const Radians& value);
void InstanceDisplay(const char* name, Handle<Image>& value);
void InstanceDisplay(const char* name, const Handle<Image>& value);
void InstanceDisplay(const char* name, Handle<Font>& value);
void InstanceDisplay(const char* name, const Handle<Font>& value);
void InstanceDisplay(const char* name, Transform& value);
void InstanceDisplay(const char* name, const Transform& value);
void InstanceDisplay(const char* name, TilemapHandle& value);
void InstanceDisplay(const char* name, const TilemapHandle& value);
void InstanceDisplay(const char* name, const Flip&);
void InstanceDisplay(const char* name, Handle<Animation>&);
void InstanceDisplay(const char* name, const Handle<Animation>&);
void InstanceDisplay(const char* name, Animation&);
void InstanceDisplay(const char* name, const Animation&);
void InstanceDisplay(const char* name, AnimationPlayer&);
void InstanceDisplay(const char* name, Entity);
void InstanceDisplay(const char* name, NullEntity);
void InstanceDisplay(const char* name, const Path&);
void InstanceDisplay(const char* name, Path&);
void InstanceDisplay(const char* name, CharacterController&);
void InstanceDisplay(const char* name, CollisionGroup&);
void InstanceDisplay(const char* name, const CollisionGroup&);
void InstanceDisplay(const char* name, const Trigger&);
void InstanceDisplay(const char* name, const Trigger::PhysicsData&);
void InstanceDisplay(const char* name, PhysicsShape&);
void InstanceDisplay(const char* name, const Color&);
void InstanceDisplay(const char* name, Color&);
void InstanceDisplay(const char* name, Image9Grid&);
void InstanceDisplay(const char* name, const Image9Grid&);

template <typename T>
void ShowSelectAssetFileDialog(Handle<T>& value,
                               const std::vector<Filter>& filters) {
    FileDialog dialog{FileDialog::Type::OpenFile};
    Path base_path = CURRENT_CONTEXT.GetProjectPath();
    dialog.SetTitle("Select Asset");
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
            value = CURRENT_CONTEXT.m_assets_manager->GetManager<T>().Load(
                relative_path);
        }
    }
}

template <typename T>
void HandleInstanceDisplayCommon(const char* name, Handle<T>& handle,
                                 const std::vector<Filter>& filters) {
    ImGui::Text("%s", name);
    if (auto filename = handle.GetFilename(); filename) {
        auto filename_str = filename->string();
        ImGui::InputText("path", filename_str.data(), filename_str.size() + 1,
                         ImGuiInputTextFlags_ReadOnly);
    }
    using payload_type = T;

    if (!handle) {
        auto& mgr = CURRENT_CONTEXT.m_assets_manager->GetManager<T>();
        if constexpr (std::is_default_constructible_v<T>) {
            if (ImGui::Button("Create")) {
                handle = mgr.Create();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Ref")) {
            ShowSelectAssetFileDialog(handle, filters);
        }
        return;
    }

    if constexpr (AssetSLInfo<payload_type>::CanEmbed) {
        bool is_embed = handle.IsEmbed();
        ImGui::SameLine();
        if (ImGui::Checkbox("embed", &is_embed)) {
            // shift to embed mode, move asset to file
            if (is_embed) {
                CURRENT_CONTEXT.m_assets_manager->GetManager<payload_type>()
                    .MakeEmbed(handle);
            } else {  // shift to un-embed mode, embed asset
                FileDialog dialog{FileDialog::Type::SaveFile};
                for (auto& filter : filters) {
                    dialog.AddFilter(filter);
                }
                dialog.SetTitle("Save asset to external file");
                dialog.Open();

                auto& files = dialog.GetSelectedFiles();
                if (!files.empty()) {
                    Path filename = files[0];
                    if (!filename.has_extension()) {
                        filename.replace_extension(
                            AssetInfoManager::GetExtension<payload_type>());
                    }
                    SaveAsset(handle.GetUUID(), *handle, filename);
                    CURRENT_CONTEXT.m_assets_manager->GetManager<T>()
                        .MakeExternal(handle, filename);
                }
            }
        }
    }

    if (!handle.IsEmbed()) {
        ImGui::SameLine();

        ImGui::PushID(handle.GetUUID());
        if (ImGui::Button("Open")) {
            std::string_view app_path = CURRENT_CONTEXT.GetAppPath();
            std::string filename = handle.GetFilename()->string();
            const char* args[] = {
                app_path.data(),
                "--filename",
                filename.c_str(),
                nullptr,
            };
            SDL_Process* process = SDL_CreateProcess(args, false);
            if (!process) {
                LOGE("failed to create asset editor process: {}",
                     SDL_GetError());
            } else {
                SDL_DestroyProcess(process);
            }
        }
        ImGui::PopID();
    }

    ImGui::SameLine();
    if (ImGui::Button("Ref")) {
        ShowSelectAssetFileDialog(handle, filters);
    }
    if (ImGui::Button("Clear")) {
        handle.Reset();
    }
}

template <typename T>
void InstanceDisplay(const char* name, std::optional<T>& value) {
    ImGui::Text("%s", name);
    ImGui::SameLine();
    ImGui::PushID(name);
    if (!value) {
        if (ImGui::Button("new")) {
            value = T{};
        }
        ImGui::Text("NULL");
    } else {
        if (ImGui::Button("del")) {
            value.reset();
        }
    }
    ImGui::PopID();

    if (value) {
        InstanceDisplay("payload", value.value());
    }
}

template <typename T>
void InstanceDisplay(const char* name, const std::optional<T>& value) {
    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    ImGui::SameLine();
    if (!value) {
        ImGui::Text("NULL");
    } else {
        InstanceDisplay("payload", value.value());
    }
    ImGui::EndDisabled();
}

template <typename T>
void InstanceDisplay(const char* name, const std::vector<T>& values) {
    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    ImGui::PushID(name);
    for (size_t i = 0; i < values.size(); i++) {
        ImGui::PushID(static_cast<int>(i));
        InstanceDisplay(std::to_string(i).c_str(), values[i]);
        ImGui::PopID();
    }
    ImGui::PopID();
    ImGui::EndDisabled();
}

template <typename T>
void InstanceDisplay(const char* name, std::vector<T>& values) {
    ImGui::Text("%s", name);
    ImGui::SameLine();
    ImGui::PushID(name);

    if (ImGui::Button("add")) {
        values.emplace_back(T{});
    }

    for (size_t i = 0; i < values.size(); i++) {
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::Button("del")) {
            values.erase(values.begin() + i);
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
        ImGui::SameLine();
        InstanceDisplay(std::to_string(i).c_str(), values[i]);
    }
    ImGui::PopID();
}

template <typename T, size_t Size>
void InstanceDisplay(const char* name, const std::array<T, Size>& values) {
    ImGui::Text("%s", name);
    for (size_t i = 0; i < values.size(); i++) {
        InstanceDisplay(std::to_string(i).c_str(), values[i]);
    }
}

template <typename T, size_t Size>
void InstanceDisplay(const char* name, std::array<T, Size>& values) {
    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    for (size_t i = 0; i < values.size(); i++) {
        InstanceDisplay(std::to_string(i).c_str(), values[i]);
    }
    ImGui::EndDisabled();
}

template <typename Key, typename Value>
void InstanceDisplay(const char* name, std::unordered_map<Key, Value>& m) {
    ImGui::Text("%s", name);
    ImGui::SameLine();
    ImGui::PushID(name);
    bool add_item = ImGui::Button("add");

    std::optional<Key> delete_key = std::nullopt;
    std::optional<std::pair<Key, Key>> rename_key = std::nullopt;

    size_t idx = 0;
    for (auto&& [key, value] : m) {
        ImGui::PushID(static_cast<int>(idx));
        if (ImGui::Button("del")) {
            delete_key = key;
            ImGui::PopID();
            break;
        }
        ImGui::SameLine();

        if (ImGui::TreeNode("item", "item %zu", idx)) {
            Key edited_key = key;
            InstanceDisplay("key", edited_key);
            InstanceDisplay("value", value);
            if (!(edited_key == key)) {
                rename_key = std::make_pair(key, std::move(edited_key));
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
        idx++;
    }

    if (delete_key) {
        m.erase(delete_key.value());
    }

    if (rename_key) {
        auto from = rename_key->first;
        auto to = rename_key->second;
        auto iter = m.find(from);
        if (iter != m.end()) {
            Value moved_value = std::move(iter->second);
            m.erase(iter);
            m[to] = std::move(moved_value);
        }
    }

    if (add_item) {
        m.emplace(Key{}, Value{});
    }
    ImGui::PopID();
}

template <typename Key, typename Value>
void InstanceDisplay(const char* name,
                     const std::unordered_map<Key, Value>& m) {
    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    size_t idx = 0;
    for (auto&& [key, value] : m) {
        ImGui::PushID(static_cast<int>(idx));
        if (ImGui::TreeNode("item", "item %zu", idx)) {
            InstanceDisplay("key", key);
            InstanceDisplay("value", value);
            ImGui::TreePop();
        }
        ImGui::PopID();
        idx++;
    }
    ImGui::EndDisabled();
}

template <typename T>
void InstanceDisplay(const char* name, const KeyFrame<T>& m) {
    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    InstanceDisplay("time", m.m_time);
    InstanceDisplay("value", m.m_value);
    ImGui::EndDisabled();
}

template <typename T>
void InstanceDisplay(const char* name, KeyFrame<T>& m) {
    ImGui::Text("%s", name);
    InstanceDisplay("time", m.m_time);
    InstanceDisplay("value", m.m_value);
}

template <typename T, AnimationTrackType TrackType>
void InstanceDisplay(const char* name, const AnimationTrack<T, TrackType>& m) {
    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    InstanceDisplay("type", m.GetType());

    auto& keyframes = m.GetKeyframes();
    if (ImGui::TreeNode("keyframes")) {
        InstanceDisplay("keyframes", keyframes);
        ImGui::TreePop();
    }
    ImGui::EndDisabled();
}

template <typename T, AnimationTrackType TrackType>
void InstanceDisplay(const char* name, AnimationTrack<T, TrackType>& m) {
    ImGui::Text("%s", name);
    InstanceDisplay("type", m.GetType());

    auto& keyframes = m.GetKeyframes();
    InstanceDisplay("keyframes", keyframes);
}

template <typename T>
void InstanceDisplay(const char* name, TVec2<T>& value) {
    ImGui::PushID(static_cast<void*>(&value));
    if constexpr (std::is_same_v<T, float>) {
        ImGui::DragFloat2(name, (float*)&value, 0.1);
    } else if constexpr (std::is_same_v<T, double>) {
        ImGui::DragScalarN(name, ImGuiDataType_Double, &value, 2, 0.1);
    } else if constexpr (std::is_same_v<T, int>) {
        ImGui::DragScalarN(name, ImGuiDataType_S32, &value, 2, 1);
    } else if constexpr (std::is_same_v<T, char>) {
        ImGui::DragScalarN(name, ImGuiDataType_S8, &value, 2, 1);
    } else if constexpr (std::is_same_v<T, short>) {
        ImGui::DragScalarN(name, ImGuiDataType_S16, &value, 2, 1);
    } else if constexpr (std::is_same_v<T, long long>) {
        ImGui::DragScalarN(name, ImGuiDataType_S64, &value, 2, 1);
    } else if constexpr (std::is_same_v<T, uint8_t>) {
        ImGui::DragScalarN(name, ImGuiDataType_U8, &value, 2, 1);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        ImGui::DragScalarN(name, ImGuiDataType_U16, &value, 2, 1);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        ImGui::DragScalarN(name, ImGuiDataType_U32, &value, 2, 1);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        ImGui::DragScalarN(name, ImGuiDataType_U64, &value, 2, 1);
    }
    ImGui::PopID();
}

template <typename T>
void InstanceDisplay(const char* name, const TVec2<T>& value) {
    ImGui::PushID(static_cast<const void*>(&value));
    ImGui::BeginDisabled(true);
    if constexpr (std::is_same_v<T, float>) {
        ImGui::DragFloat2(name, (float*)&value, 0.1);
    } else if constexpr (std::is_same_v<T, double>) {
        ImGui::DragScalarN(name, ImGuiDataType_Double, (void*)&value, 2, 0.1);
    } else if constexpr (std::is_same_v<T, int>) {
        ImGui::DragScalarN(name, ImGuiDataType_S32, (void*)&value, 2, 1);
    } else if constexpr (std::is_same_v<T, char>) {
        ImGui::DragScalarN(name, ImGuiDataType_S8, (void*)&value, 2, 1);
    } else if constexpr (std::is_same_v<T, short>) {
        ImGui::DragScalarN(name, ImGuiDataType_S16, (void*)&value, 2, 1);
    } else if constexpr (std::is_same_v<T, long long>) {
        ImGui::DragScalarN(name, ImGuiDataType_S64, (void*)&value, 2, 1);
    } else if constexpr (std::is_same_v<T, uint8_t>) {
        ImGui::DragScalarN(name, ImGuiDataType_U8, (void*)&value, 2, 1);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        ImGui::DragScalarN(name, ImGuiDataType_U16, (void*)&value, 2, 1);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        ImGui::DragScalarN(name, ImGuiDataType_U32, (void*)&value, 2, 1);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        ImGui::DragScalarN(name, ImGuiDataType_U64, (void*)&value, 2, 1);
    }
    ImGui::EndDisabled();
    ImGui::PopID();
}
