#pragma once
#include <cstdint>
#include "imgui.h"

class ImGuiIDGenerator {
public:
    static int Gen() { return id++; }

private:
    static int id;
};

#define IMGUI_PUSHID(value) { ImGui::PushID((void*)&value); }
#define IMGUI_POPID() ImGui::PopID()
