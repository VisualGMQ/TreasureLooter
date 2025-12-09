#pragma once
#include <cstdint>
#include "imgui.h"

class ImGuiIDGenerator {
public:
    static int Gen() { return id++; }

private:
    static int id;
};

#define IMGUI_PUSHID() { static int id = ImGuiIDGenerator::Gen(); ImGui::PushID(id); }
#define IMGUI_POPID() ImGui::PopID()
