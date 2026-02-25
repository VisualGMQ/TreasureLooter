#include "engine/script/script_imgui_binding.hpp"
#include "engine/log.hpp"
#include "engine/macros.hpp"
#include "imgui.h"

#include "lua.h"
#include "lualib.h"
#include "LuaBridge/LuaBridge.h"

static constexpr size_t kImGuiInputTextBufSize = 4096;

static int ImGui_Checkbox(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    bool v = lua_toboolean(L, 2) != 0;
    bool changed = ImGui::Checkbox(label, &v);
    lua_pushboolean(L, v);
    lua_pushboolean(L, changed);
    return 2;
}

static int ImGui_InputText(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    size_t len = 0;
    const char* s = lua_tolstring(L, 2, &len);
    char buf[kImGuiInputTextBufSize];
    if (s && len < sizeof(buf))
        memcpy(buf, s, len + 1);
    else
        buf[0] = '\0';
    bool changed = ImGui::InputText(label, buf, sizeof(buf));
    lua_pushstring(L, buf);
    lua_pushboolean(L, changed);
    return 2;
}

static int ImGui_InputInt(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    int v = static_cast<int>(luaL_checkinteger(L, 2));
    bool changed = ImGui::InputInt(label, &v);
    lua_pushinteger(L, v);
    lua_pushboolean(L, changed);
    return 2;
}

static int ImGui_InputFloat(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    float v = static_cast<float>(luaL_checknumber(L, 2));
    bool changed = ImGui::InputFloat(label, &v);
    lua_pushnumber(L, v);
    lua_pushboolean(L, changed);
    return 2;
}

static int ImGui_DragFloat(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    float v = static_cast<float>(luaL_checknumber(L, 2));
    float speed = lua_gettop(L) >= 3 ? static_cast<float>(lua_tonumber(L, 3)) : 1.0f;
    float v_min = lua_gettop(L) >= 4 ? static_cast<float>(lua_tonumber(L, 4)) : 0.0f;
    float v_max = lua_gettop(L) >= 5 ? static_cast<float>(lua_tonumber(L, 5)) : 0.0f;
    bool changed = ImGui::DragFloat(label, &v, speed, v_min, v_max);
    lua_pushnumber(L, v);
    lua_pushboolean(L, changed);
    return 2;
}

static int ImGui_DragInt(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    int v = static_cast<int>(luaL_checkinteger(L, 2));
    float speed = lua_gettop(L) >= 3 ? static_cast<float>(lua_tonumber(L, 3)) : 1.0f;
    int v_min = lua_gettop(L) >= 4 ? static_cast<int>(lua_tointeger(L, 4)) : 0;
    int v_max = lua_gettop(L) >= 5 ? static_cast<int>(lua_tointeger(L, 5)) : 0;
    bool changed = ImGui::DragInt(label, &v, speed, v_min, v_max);
    lua_pushinteger(L, v);
    lua_pushboolean(L, changed);
    return 2;
}

static int ImGui_SliderFloat(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    float v = static_cast<float>(luaL_checknumber(L, 2));
    float v_min = static_cast<float>(luaL_checknumber(L, 3));
    float v_max = static_cast<float>(luaL_checknumber(L, 4));
    bool changed = ImGui::SliderFloat(label, &v, v_min, v_max);
    lua_pushnumber(L, v);
    lua_pushboolean(L, changed);
    return 2;
}

static int ImGui_SliderInt(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    int v = static_cast<int>(luaL_checkinteger(L, 2));
    int v_min = static_cast<int>(luaL_checkinteger(L, 3));
    int v_max = static_cast<int>(luaL_checkinteger(L, 4));
    bool changed = ImGui::SliderInt(label, &v, v_min, v_max);
    lua_pushinteger(L, v);
    lua_pushboolean(L, changed);
    return 2;
}

static int ImGui_PushID(lua_State* L) {
    if (lua_type(L, 1) == LUA_TNUMBER)
        ImGui::PushID(static_cast<int>(lua_tointeger(L, 1)));
    else
        ImGui::PushID(luaL_checkstring(L, 1));
    return 0;
}

static int ImGui_BeginDisabled(lua_State* L) {
    bool disabled = (lua_gettop(L) == 0 || lua_toboolean(L, 1));
    ImGui::BeginDisabled(disabled);
    return 0;
}

void bindImGui(lua_State* L) {
    TL_RETURN_IF_NULL_WITH_LOG(L, LOGE, "lua_State* is null!");

    luabridge::getGlobalNamespace(L)
        .beginNamespace("ImGui")
        // Window
        .addFunction("Begin", +[](const char* name) { return ImGui::Begin(name); })
        .addFunction("End", +[]() { ImGui::End(); })
        // Text
        .addFunction("Text", +[](const char* text) { ImGui::TextUnformatted(text); })
        .addFunction("SeparatorText", +[](const char* label) { ImGui::SeparatorText(label); })
        // Main
        .addFunction("Button", +[](const char* label) { return ImGui::Button(label); })
        .addFunction("Checkbox", &ImGui_Checkbox)
        .addFunction("InputText", &ImGui_InputText)
        .addFunction("InputInt", &ImGui_InputInt)
        .addFunction("InputFloat", &ImGui_InputFloat)
        .addFunction("DragFloat", &ImGui_DragFloat)
        .addFunction("DragInt", &ImGui_DragInt)
        .addFunction("SliderFloat", &ImGui_SliderFloat)
        .addFunction("SliderInt", &ImGui_SliderInt)
        // Tree
        .addFunction("TreeNode", +[](const char* label) { return ImGui::TreeNode(label); })
        .addFunction("TreePop", +[]() { ImGui::TreePop(); })
        .addFunction("CollapsingHeader", +[](const char* label) { return ImGui::CollapsingHeader(label); })
        // Layout
        .addFunction("SameLine", +[]() { ImGui::SameLine(); })
        .addFunction("Separator", +[]() { ImGui::Separator(); })
        .addFunction("NewLine", +[]() { ImGui::NewLine(); })
        .addFunction("Spacing", +[]() { ImGui::Spacing(); })
        .addFunction("Indent", +[]() { ImGui::Indent(); })
        .addFunction("Unindent", +[]() { ImGui::Unindent(); })
        // ID
        .addFunction("PushID", &ImGui_PushID)
        .addFunction("PopID", +[]() { ImGui::PopID(); })
        // Disable
        .addFunction("BeginDisabled", &ImGui_BeginDisabled)
        .addFunction("EndDisabled", +[]() { ImGui::EndDisabled(); })
        // Popup
        .addFunction("BeginPopup", +[](const char* str_id) { return ImGui::BeginPopup(str_id); })
        .addFunction("EndPopup", +[]() { ImGui::EndPopup(); })
        .addFunction("OpenPopup", +[](const char* str_id) { ImGui::OpenPopup(str_id); })
        .addFunction("CloseCurrentPopup", +[]() { ImGui::CloseCurrentPopup(); })
        // Tooltip
        .addFunction("BeginTooltip", +[]() { return ImGui::BeginTooltip(); })
        .addFunction("EndTooltip", +[]() { ImGui::EndTooltip(); })
        .addFunction("SetTooltip", +[](const char* fmt) { ImGui::SetTooltip("%s", fmt); })
        // Item query
        .addFunction("IsItemHovered", +[]() { return ImGui::IsItemHovered(); })
        .addFunction("IsItemClicked", +[]() { return ImGui::IsItemClicked(); })
        // 常用枚举常量
        .addProperty("WindowFlags_None", +[]() { return static_cast<int>(ImGuiWindowFlags_None); })
        .addProperty("WindowFlags_NoTitleBar", +[]() { return static_cast<int>(ImGuiWindowFlags_NoTitleBar); })
        .addProperty("WindowFlags_NoResize", +[]() { return static_cast<int>(ImGuiWindowFlags_NoResize); })
        .addProperty("WindowFlags_NoMove", +[]() { return static_cast<int>(ImGuiWindowFlags_NoMove); })
        .addProperty("WindowFlags_NoScrollbar", +[]() { return static_cast<int>(ImGuiWindowFlags_NoScrollbar); })
        .addProperty("WindowFlags_NoCollapse", +[]() { return static_cast<int>(ImGuiWindowFlags_NoCollapse); })
        .addProperty("WindowFlags_AlwaysAutoResize", +[]() { return static_cast<int>(ImGuiWindowFlags_AlwaysAutoResize); })
        .addProperty("WindowFlags_NoBackground", +[]() { return static_cast<int>(ImGuiWindowFlags_NoBackground); })
        .addProperty("WindowFlags_NoSavedSettings", +[]() { return static_cast<int>(ImGuiWindowFlags_NoSavedSettings); })
        .addProperty("WindowFlags_MenuBar", +[]() { return static_cast<int>(ImGuiWindowFlags_MenuBar); })
        .addProperty("WindowFlags_NoDecoration", +[]() { return static_cast<int>(ImGuiWindowFlags_NoDecoration); })
        .addProperty("WindowFlags_NoInputs", +[]() { return static_cast<int>(ImGuiWindowFlags_NoInputs); })
        .addProperty("Cond_None", +[]() { return static_cast<int>(ImGuiCond_None); })
        .addProperty("Cond_Always", +[]() { return static_cast<int>(ImGuiCond_Always); })
        .addProperty("Cond_Once", +[]() { return static_cast<int>(ImGuiCond_Once); })
        .addProperty("Cond_FirstUseEver", +[]() { return static_cast<int>(ImGuiCond_FirstUseEver); })
        .addProperty("Cond_Appearing", +[]() { return static_cast<int>(ImGuiCond_Appearing); })
        .addProperty("TreeNodeFlags_None", +[]() { return static_cast<int>(ImGuiTreeNodeFlags_None); })
        .addProperty("TreeNodeFlags_Selected", +[]() { return static_cast<int>(ImGuiTreeNodeFlags_Selected); })
        .addProperty("TreeNodeFlags_Framed", +[]() { return static_cast<int>(ImGuiTreeNodeFlags_Framed); })
        .addProperty("TreeNodeFlags_DefaultOpen", +[]() { return static_cast<int>(ImGuiTreeNodeFlags_DefaultOpen); })
        .addProperty("TreeNodeFlags_Leaf", +[]() { return static_cast<int>(ImGuiTreeNodeFlags_Leaf); })
        .addProperty("TreeNodeFlags_Bullet", +[]() { return static_cast<int>(ImGuiTreeNodeFlags_Bullet); })
        .addProperty("TreeNodeFlags_CollapsingHeader", +[]() { return static_cast<int>(ImGuiTreeNodeFlags_CollapsingHeader); })
        .addProperty("InputTextFlags_None", +[]() { return static_cast<int>(ImGuiInputTextFlags_None); })
        .addProperty("InputTextFlags_CharsDecimal", +[]() { return static_cast<int>(ImGuiInputTextFlags_CharsDecimal); })
        .addProperty("InputTextFlags_CharsHexadecimal", +[]() { return static_cast<int>(ImGuiInputTextFlags_CharsHexadecimal); })
        .addProperty("InputTextFlags_ReadOnly", +[]() { return static_cast<int>(ImGuiInputTextFlags_ReadOnly); })
        .addProperty("InputTextFlags_Password", +[]() { return static_cast<int>(ImGuiInputTextFlags_Password); })
        .addProperty("InputTextFlags_AutoSelectAll", +[]() { return static_cast<int>(ImGuiInputTextFlags_AutoSelectAll); })
        .addProperty("SliderFlags_None", +[]() { return static_cast<int>(ImGuiSliderFlags_None); })
        .addProperty("SliderFlags_Logarithmic", +[]() { return static_cast<int>(ImGuiSliderFlags_Logarithmic); })
        .addProperty("SliderFlags_NoRoundToFormat", +[]() { return static_cast<int>(ImGuiSliderFlags_NoRoundToFormat); })
        .addProperty("SliderFlags_NoInput", +[]() { return static_cast<int>(ImGuiSliderFlags_NoInput); })
        .addProperty("SliderFlags_AlwaysClamp", +[]() { return static_cast<int>(ImGuiSliderFlags_AlwaysClamp); })
        .addProperty("PopupFlags_None", +[]() { return static_cast<int>(ImGuiPopupFlags_None); })
        .addProperty("PopupFlags_MouseButtonLeft", +[]() { return static_cast<int>(ImGuiPopupFlags_MouseButtonLeft); })
        .addProperty("PopupFlags_MouseButtonRight", +[]() { return static_cast<int>(ImGuiPopupFlags_MouseButtonRight); })
        .addProperty("PopupFlags_MouseButtonMiddle", +[]() { return static_cast<int>(ImGuiPopupFlags_MouseButtonMiddle); })
        .addProperty("PopupFlags_NoReopen", +[]() { return static_cast<int>(ImGuiPopupFlags_NoReopen); })
        .addProperty("PopupFlags_NoOpenOverExistingPopup", +[]() { return static_cast<int>(ImGuiPopupFlags_NoOpenOverExistingPopup); })
        .addProperty("HoveredFlags_None", +[]() { return static_cast<int>(ImGuiHoveredFlags_None); })
        .addProperty("HoveredFlags_ChildWindows", +[]() { return static_cast<int>(ImGuiHoveredFlags_ChildWindows); })
        .addProperty("HoveredFlags_RootWindow", +[]() { return static_cast<int>(ImGuiHoveredFlags_RootWindow); })
        .addProperty("HoveredFlags_AnyWindow", +[]() { return static_cast<int>(ImGuiHoveredFlags_AnyWindow); })
        .addProperty("HoveredFlags_AllowWhenBlockedByPopup", +[]() { return static_cast<int>(ImGuiHoveredFlags_AllowWhenBlockedByPopup); })
        .addProperty("HoveredFlags_AllowWhenDisabled", +[]() { return static_cast<int>(ImGuiHoveredFlags_AllowWhenDisabled); })
        .addProperty("HoveredFlags_ForTooltip", +[]() { return static_cast<int>(ImGuiHoveredFlags_ForTooltip); })
        .addProperty("FocusedFlags_None", +[]() { return static_cast<int>(ImGuiFocusedFlags_None); })
        .addProperty("FocusedFlags_ChildWindows", +[]() { return static_cast<int>(ImGuiFocusedFlags_ChildWindows); })
        .addProperty("FocusedFlags_RootWindow", +[]() { return static_cast<int>(ImGuiFocusedFlags_RootWindow); })
        .addProperty("FocusedFlags_AnyWindow", +[]() { return static_cast<int>(ImGuiFocusedFlags_AnyWindow); })
        .addProperty("SelectableFlags_None", +[]() { return static_cast<int>(ImGuiSelectableFlags_None); })
        .addProperty("SelectableFlags_NoAutoClosePopups", +[]() { return static_cast<int>(ImGuiSelectableFlags_NoAutoClosePopups); })
        .addProperty("SelectableFlags_SpanAllColumns", +[]() { return static_cast<int>(ImGuiSelectableFlags_SpanAllColumns); })
        .addProperty("SelectableFlags_AllowDoubleClick", +[]() { return static_cast<int>(ImGuiSelectableFlags_AllowDoubleClick); })
        .addProperty("SelectableFlags_Disabled", +[]() { return static_cast<int>(ImGuiSelectableFlags_Disabled); })
        .addProperty("ComboFlags_None", +[]() { return static_cast<int>(ImGuiComboFlags_None); })
        .addProperty("ComboFlags_PopupAlignLeft", +[]() { return static_cast<int>(ImGuiComboFlags_PopupAlignLeft); })
        .addProperty("ComboFlags_HeightSmall", +[]() { return static_cast<int>(ImGuiComboFlags_HeightSmall); })
        .addProperty("ComboFlags_HeightRegular", +[]() { return static_cast<int>(ImGuiComboFlags_HeightRegular); })
        .addProperty("ComboFlags_HeightLarge", +[]() { return static_cast<int>(ImGuiComboFlags_HeightLarge); })
        .addProperty("ComboFlags_NoArrowButton", +[]() { return static_cast<int>(ImGuiComboFlags_NoArrowButton); })
        .addProperty("ChildFlags_None", +[]() { return static_cast<int>(ImGuiChildFlags_None); })
        .addProperty("ChildFlags_Borders", +[]() { return static_cast<int>(ImGuiChildFlags_Borders); })
        .addProperty("ChildFlags_FrameStyle", +[]() { return static_cast<int>(ImGuiChildFlags_FrameStyle); })
        .addProperty("Dir_None", +[]() { return static_cast<int>(ImGuiDir_None); })
        .addProperty("Dir_Left", +[]() { return static_cast<int>(ImGuiDir_Left); })
        .addProperty("Dir_Right", +[]() { return static_cast<int>(ImGuiDir_Right); })
        .addProperty("Dir_Up", +[]() { return static_cast<int>(ImGuiDir_Up); })
        .addProperty("Dir_Down", +[]() { return static_cast<int>(ImGuiDir_Down); })
        .addProperty("MouseButton_Left", +[]() { return static_cast<int>(ImGuiMouseButton_Left); })
        .addProperty("MouseButton_Right", +[]() { return static_cast<int>(ImGuiMouseButton_Right); })
        .addProperty("MouseButton_Middle", +[]() { return static_cast<int>(ImGuiMouseButton_Middle); })
        .endNamespace();
}
