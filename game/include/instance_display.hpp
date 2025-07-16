#pragma once
#include "context.hpp"
#include "imgui.h"
#include "imgui_id_generator.hpp"

#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct Vec2;
struct Region;
struct Degrees;
struct Radians;
struct Pose;
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
void InstanceDisplay(const char* name, Vec2& value);
void InstanceDisplay(const char* name, const Vec2& value);
void InstanceDisplay(const char* name, Region& value);
void InstanceDisplay(const char* name, const Region& value);
void InstanceDisplay(const char* name, Degrees& value);
void InstanceDisplay(const char* name, const Degrees& value);
void InstanceDisplay(const char* name, Radians& value);
void InstanceDisplay(const char* name, const Radians& value);
void InstanceDisplay(const char* name, Image* value);
void InstanceDisplay(const char* name, const Image* value);
void InstanceDisplay(const char* name, Pose& value);
void InstanceDisplay(const char* name, const Pose& value);

template <typename T>
void InstanceDisplay(const char* name, std::optional<T>& value) {
    ImGui::Text("%s", name);
    ImGui::SameLine();
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
    ImGui::Text(name);
    for (size_t i = 0; i < values.size(); i++) {
        ImGui::PushID(ImGuiIDGenerator::Gen());
        InstanceDisplay(std::to_string(i).c_str(), values[i]);
        ImGui::PopID();
    }
    ImGui::EndDisabled();
}

template <typename T>
void InstanceDisplay(const char* name, std::vector<T>& values) {
    ImGui::Text(name);
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
    ImGui::Text(name);
    for (size_t i = 0; i < values.size(); i++) {
        InstanceDisplay(std::to_string(i).c_str(), values[i]);
    }
}

template <typename T, size_t Size>
void InstanceDisplay(const char* name, std::array<T, Size>& values) {
    ImGui::BeginDisabled(true);
    ImGui::Text(name);
    for (size_t i = 0; i < values.size(); i++) {
        InstanceDisplay(std::to_string(i).c_str(), values[i]);
    }
    ImGui::EndDisabled();
}

template <typename Key, typename Value>
void InstanceDisplay(const char* name, std::unordered_map<Key, Value>& m) {
    ImGui::Text(name);
    for (auto&& [key, value] : m) {
        InstanceDisplay("Key", key);
        InstanceDisplay("Val", value);
    }
}

template <typename Key, typename Value>
void InstanceDisplay(const char* name,
                     const std::unordered_map<Key, Value>& m) {
    ImGui::BeginDisabled(true);
    ImGui::Text(name);
    for (auto&& [key, value] : m) {
        InstanceDisplay("Key", key);
        InstanceDisplay("Val", value);
    }
    ImGui::EndDisabled();
}

void InstanceDisplay(const char* name, const Flip&);
