#pragma once
#include "context.hpp"
#include "imgui.h"
#include "engine/imgui_id_generator.hpp"
#include "engine/math.hpp"

#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

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
void InstanceDisplay(const char* name, const PhysicsActor&);
void InstanceDisplay(const char* name, const Color&);
void InstanceDisplay(const char* name, Color&);
void InstanceDisplay(const char* name, Image9Grid&);
void InstanceDisplay(const char* name, const Image9Grid&);

template <typename T>
void InstanceDisplay(const char* name, std::optional<T>& value) {
    ImGui::Text("%s", name);
    ImGui::SameLine();
    if (!value) {
        ImGui::PushID(ImGuiIDGenerator::Gen());
        if (ImGui::Button("new")) {
            value = T{};
        }
        ImGui::PopID();
        ImGui::Text("NULL");
    } else {
        ImGui::PushID(ImGuiIDGenerator::Gen());
        if (ImGui::Button("del")) {
            value.reset();
        }
        ImGui::PopID();
    }

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
    for (size_t i = 0; i < values.size(); i++) {
        ImGui::PushID(ImGuiIDGenerator::Gen());
        InstanceDisplay(std::to_string(i).c_str(), values[i]);
        ImGui::PopID();
    }
    ImGui::EndDisabled();
}

template <typename T>
void InstanceDisplay(const char* name, std::vector<T>& values) {
    ImGui::Text("%s", name);
    ImGui::SameLine();

    ImGui::PushID(ImGuiIDGenerator::Gen());
    if (ImGui::Button("add")) {
        values.emplace_back(T{});
    }
    ImGui::PopID();

    for (size_t i = 0; i < values.size(); i++) {
        ImGui::PushID(ImGuiIDGenerator::Gen());
        if (ImGui::Button("del")) {
            values.erase(values.begin() + i);
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
        ImGui::SameLine();
        InstanceDisplay(std::to_string(i).c_str(), values[i]);
    }
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
    for (auto&& [key, value] : m) {
        InstanceDisplay("Key", key);
        InstanceDisplay("Val", value);
    }
}

template <typename Key, typename Value>
void InstanceDisplay(const char* name,
                     const std::unordered_map<Key, Value>& m) {
    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    for (auto&& [key, value] : m) {
        InstanceDisplay("Key", key);
        InstanceDisplay("Val", value);
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
    ImGui::PushID(ImGuiIDGenerator::Gen());
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
    ImGui::PushID(ImGuiIDGenerator::Gen());
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
