#include "client/debug_panel.hpp"
#include "client/context.hpp"
#include "client/hfsm.hpp"
#include "client/input/input.hpp"
#include "common/context.hpp"
#include "common/entity_name_manager.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "imgui.h"
#include <charconv>
#include <type_traits>

#define FTS_FUZZY_MATCH_IMPLEMENTATION
#include "common/asset_manager.hpp"
#include "common/scene.hpp"
#include "fts_fuzzy_match.hpp"

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <utility>

namespace internal {

ParamConvertError::ParamConvertError(const std::string& error_msg)
    : m_error{true}, m_err_msg{error_msg} {}

ParamConvertError::operator bool() const {
    return !m_error;
}

std::string_view ParamConvertError::GetErrMsg() const {
    return m_err_msg;
}

ParamConvertError ConvertParam(std::string_view text, Entity& out_entity) {
    constexpr std::string_view entity_prefix = "Entity<";
    std::underlying_type_t<Entity> entity_number;
    ParamConvertError error{fmt::format("convert entity from {} failed", text)};

    if (text.size() > entity_prefix.size()) {
        auto idx = text.find('>', entity_prefix.size());
        if (idx != std::string_view::npos) {
            auto result = std::from_chars(text.data() + entity_prefix.size(),
                                          text.data() + idx, entity_number);
            TL_RETURN_VALUE_IF_FALSE(result.ec == std::errc{}, error);
            out_entity = static_cast<Entity>(entity_number);
            return {};
        }
    }

    return error;
}

ParamConvertError ConvertParam(std::string_view text, bool& out_boolean) {
    if (text == "ON" || text == "on" || text == "true" || text == "TRUE" ||
        text == "True") {
        out_boolean = true;
        return {};
    }
    if (text == "OFF" || text == "FALSE" || text == "false" ||
        text == "False" || text == "off") {
        out_boolean = false;
        return {};
    }

    return ParamConvertError{fmt::format("can't convert {} to boolean", text)};
}

ParamConvertError ConvertParam(std::string_view text, std::string& out_value) {
    out_value = text;
    return {};
}

template <>
std::vector<std::string> GetDebugPanelParamHint<Entity>() {
    static std::vector<std::string> result;
    result.clear();

    auto scene = COMMON_CONTEXT.m_scene_manager->GetCurrentScene();
    TL_RETURN_VALUE_IF_NULL(scene, result);

    auto& entities = scene->GetAllEntities();

    for (auto& entity : entities) {
        result.push_back(
            "Entity<" +
            std::to_string(
                static_cast<std::underlying_type_t<Entity>>(entity)) +
            ">");
    }

    return result;
}

template <>
std::vector<std::string> GetDebugPanelParamHint<bool>() {
    static std::vector<std::string> result = {"true", "false"};
    return result;
}

template <>
std::vector<std::string> GetDebugPanelParamHint<std::string>() {
    static std::vector<std::string> result;
    return result;
}

template <>
std::vector<std::string> GetDebugPanelParamHint<float>() {
    static std::vector<std::string> result;
    return result;
}

template <>
std::vector<std::string> GetDebugPanelParamHint<double>() {
    static std::vector<std::string> result;
    return result;
}

}  // namespace internal

void DebugPanel::RegisterCmd(
    const std::string& name, luabridge::LuaRef cmd,
    const std::vector<luabridge::LuaRef>& param_hints) {
    if (!cmd.isCallable()) {
        LOGE("[DebugPanel] RegisterCmd from luau '{}' is not callable", name);
        return;
    }

    if (m_commands.find(name) == m_commands.end()) {
        m_cmd_names.push_back(name);
    }

    DebugCmd debug_cmd;
    debug_cmd.m_fn = [=](const std::vector<std::string>& args) {
        auto result = cmd(args);
        if (result.errorCode()) {
            LOGE("[DebugPanel] lua command '{}' error: {}", name,
                 result.errorMessage());
        }
    };
    for (auto& hint : param_hints) {
        debug_cmd.m_param_hints.emplace_back([=]() -> std::vector<std::string> {
            auto lua_call_result = call(hint);
            std::vector<std::string> result;

            TL_RETURN_VALUE_IF_FALSE_WITH_LOG(
                !lua_call_result.errorCode(), result, LOGE,
                "[DebugPanel] lua command '{}' hint error: {}", name,
                lua_call_result.errorMessage());

            TL_RETURN_VALUE_IF_FALSE_WITH_LOG(
                lua_call_result.size() > 0, result, LOGE,
                "[DebugPanel] lua command '{}' param hint function "
                "must return a table of strings", name);

            const luabridge::LuaRef table = lua_call_result[0];
            TL_RETURN_VALUE_IF_FALSE_WITH_LOG(
                table.isTable(), result, LOGE,
                "[DebugPanel] lua command '{}' param hint function "
                "must return a table of strings", name);

            for (int i = 1; i <= table.length(); ++i) {
                const auto elem = table[i];
                TL_RETURN_DEFAULT_IF_FALSE_WITH_LOG(
                    elem.isString(), LOGE,
                    "lua param hint function not return "
                    "std::vector<std::string> in param {}",
                    i - 1);
                result.emplace_back(elem.tostring());
            }
            return result;
        });
    }

    m_commands.try_emplace(name, std::move(debug_cmd));
}

std::vector<std::string> DebugPanel::collectFuzzyMatches(
    const std::string& name) const {
    return collectFuzzyMatchesFrom(name, m_cmd_names);
}

std::vector<std::string> DebugPanel::collectFuzzyMatchesFrom(
    const std::string& pattern, const std::vector<std::string>& items) const {
    std::vector<std::pair<int, const std::string*>> scored;

    if (pattern.empty()) {
        for (const auto& item : items) {
            scored.emplace_back(0, &item);
        }
    } else {
        for (const auto& item : items) {
            int score = 0;
            if (fts::fuzzy_match(pattern.c_str(), item.c_str(), score)) {
                scored.emplace_back(score, &item);
            }
        }
    }

    std::sort(scored.begin(), scored.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    std::vector<std::string> result;
    result.reserve(scored.size());
    for (const auto& [score, ptr] : scored) {
        result.push_back(*ptr);
    }
    return result;
}

std::vector<std::string> DebugPanel::collectCandidates(
    const std::string& input) const {
    const std::vector<std::string> tokens = splitTokens(input);
    if (tokens.empty()) {
        return collectFuzzyMatches("");
    }

    const bool ends_with_space =
        !input.empty() && (input.back() == ' ' || input.back() == '\t');
    const std::string& cmd_name = tokens.front();
    const auto cmd_it = m_commands.find(cmd_name);
    if (cmd_it == m_commands.end()) {
        return collectFuzzyMatches(cmd_name);
    }

    const auto& hints = cmd_it->second.m_param_hints;
    if (hints.empty()) {
        return {};
    }

    size_t param_index = 0;
    std::string partial;
    if (ends_with_space) {
        param_index = tokens.size() - 1;
    } else if (tokens.size() == 1) {
        param_index = 0;
    } else {
        param_index = tokens.size() - 2;
        partial = tokens.back();
    }

    if (param_index >= hints.size()) {
        return {};
    }

    const std::vector<std::string> all = hints[param_index]();
    for (const auto& item : all) {
        if (item == partial) {
            return {};
        }
    }
    return collectFuzzyMatchesFrom(partial, all);
}

const std::string& DebugPanel::FindCmdFuzzy(const std::string& name) {
    auto matches = collectFuzzyMatches(name);
    if (matches.empty()) {
        m_fuzzy_result.clear();
    } else {
        m_fuzzy_result = matches.front();
    }
    return m_fuzzy_result;
}

void DebugPanel::ExecuteCmd(const std::string& name,
                            const std::vector<std::string>& args) {
    auto it = m_commands.find(name);
    TL_RETURN_IF_FALSE(it != m_commands.end());

    it->second.m_fn(args);
}

void DebugPanel::refreshCandidates() {
    if (m_last_input == m_input) {
        return;
    }

    m_last_input = m_input;
    m_candidates = collectCandidates(m_input);
    m_selected = m_candidates.empty() ? -1 : 0;
    m_scroll_selected = -1;
    m_history_cursor = -1;
    m_error_msg.clear();
}

void DebugPanel::moveSelection(int dir) {
    if (m_candidates.empty()) {
        return;
    }

    const int n = static_cast<int>(m_candidates.size());
    if (m_selected < 0) {
        m_selected = dir > 0 ? 0 : n - 1;
    } else {
        m_selected = ((m_selected + dir) % n + n) % n;
    }
}

std::vector<std::string> DebugPanel::splitTokens(const std::string& input) {
    std::vector<std::string> tokens;
    std::string cur;
    for (char c : input) {
        if (c == ' ' || c == '\t') {
            if (!cur.empty()) {
                tokens.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) {
        tokens.push_back(cur);
    }
    return tokens;
}

void DebugPanel::submitCommand(const std::string& name) {
    const std::vector<std::string> tokens = splitTokens(name);
    const std::string cmd_name =
        tokens.empty() ? std::string() : tokens.front();
    const bool valid = m_commands.find(cmd_name) != m_commands.end();
    m_history.push_back({name, valid});
    m_scroll_to_bottom = true;

    if (valid) {
        const std::vector<std::string> args(tokens.begin() + 1, tokens.end());
        ExecuteCmd(cmd_name, args);
    }
}

bool DebugPanel::TrySubmitCommand(const std::string& input) {
    const std::vector<std::string> tokens = splitTokens(input);
    if (tokens.empty()) {
        return false;
    }

    const auto cmd_it = m_commands.find(tokens.front());
    if (cmd_it != m_commands.end()) {
        const size_t expected = cmd_it->second.m_param_hints.size();
        const size_t provided = tokens.size() - 1;
        if (expected > 0 && provided < expected) {
            m_error_msg = fmt::format("no enough param, require {} params",
                                      expected);
            return false;
        }
    }

    submitCommand(input);
    return true;
}

std::string DebugPanel::BuildFillText(const std::string& input,
                                      const std::string& candidate) {
    const std::vector<std::string> tokens = splitTokens(input);
    if (tokens.empty()) {
        return candidate;
    }

    const bool cmd_exact = m_commands.find(tokens.front()) != m_commands.end();
    if (!cmd_exact) {
        return candidate;
    }

    const bool ends_with_space =
        !input.empty() && (input.back() == ' ' || input.back() == '\t');
    size_t keep = tokens.size();
    if (!ends_with_space && tokens.size() >= 2) {
        keep = tokens.size() - 1;
    }

    std::string result;
    for (size_t i = 0; i < keep; ++i) {
        if (!result.empty()) {
            result += ' ';
        }
        result += tokens[i];
    }
    if (!result.empty()) {
        result += ' ';
    }
    result += candidate;
    return result;
}

int DebugPanel::inputTextCallback(ImGuiInputTextCallbackData* data) {
    auto* self = static_cast<DebugPanel*>(data->UserData);
    return self->handleInputCallback(data);
}

int DebugPanel::handleInputCallback(ImGuiInputTextCallbackData* data) {
    switch (data->EventFlag) {
        case ImGuiInputTextFlags_CallbackAlways: {
            if (m_pending_fill) {
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, m_pending_fill_text.c_str());
                data->CursorPos = data->BufTextLen;
                data->SelectionStart = data->BufTextLen;
                data->SelectionEnd = data->BufTextLen;
                data->BufDirty = true;
                m_pending_fill = false;
            }
        } break;
        case ImGuiInputTextFlags_CallbackHistory: {
            const int dir = data->EventKey == ImGuiKey_UpArrow ? -1 : 1;
            if (!m_candidates.empty()) {
                moveSelection(dir);
            } else if (!m_history.empty()) {
                const int n = static_cast<int>(m_history.size());
                if (m_history_cursor < 0) {
                    m_history_cursor = dir < 0 ? n - 1 : n;
                } else {
                    m_history_cursor += dir;
                }
                if (m_history_cursor < 0) {
                    m_history_cursor = 0;
                }
                if (m_history_cursor > n) {
                    m_history_cursor = n;
                }

                std::string text;
                if (m_history_cursor >= 0 && m_history_cursor < n) {
                    text = m_history[m_history_cursor].m_command;
                }

                data->DeleteChars(0, data->BufTextLen);
                if (!text.empty()) {
                    data->InsertChars(0, text.c_str());
                }
                m_last_input = text;
                m_candidates.clear();
                m_selected = -1;
            }
        } break;
        default:
            break;
    }
    return 0;
}

void DebugPanel::Update() {
    TL_RETURN_IF_FALSE(CLIENT_CONTEXT.m_input_manager);
    InputManager& input = *CLIENT_CONTEXT.m_input_manager;

    // Re-enable first so the toggle action is always readable, even while the
    // panel is open and gameplay input is otherwise suppressed.
    input.Enable();

    const Action& action = input.GetAction("ShowDebugPanel");
    if (action.IsPressed()) {
        m_visible = !m_visible;
    }

    // While the panel is open, suppress all gameplay input.
    if (m_visible) {
        input.Disable();
    }
}

void DebugPanel::Render() {
    TL_RETURN_IF_FALSE(m_visible);

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowBgAlpha(0.4f);

    const ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavInputs;

    if (!ImGui::Begin("##debug_panel", nullptr, window_flags)) {
        ImGui::End();
        return;
    }

    refreshCandidates();

    constexpr int kMaxCandidateRows = 10;
    const ImGuiStyle& style = ImGui::GetStyle();
    const float row_height = ImGui::GetFrameHeight();
    const size_t candidate_count = m_candidates.size();
    const bool candidates_scrollable = candidate_count > kMaxCandidateRows;
    const int visible_rows =
        candidates_scrollable ? kMaxCandidateRows
                              : static_cast<int>(candidate_count);
    const float inner_rows_height =
        static_cast<float>(visible_rows) * row_height +
        static_cast<float>(std::max(0, visible_rows - 1)) *
            style.ItemSpacing.y;
    const float candidates_height =
        candidate_count == 0
            ? 0.0f
            : inner_rows_height + style.ChildBorderSize * 2.0f +
                  style.WindowPadding.y * 2.0f + 1.0f;
    const float error_height = m_error_msg.empty()
                                   ? 0.0f
                                   : ImGui::GetTextLineHeightWithSpacing();
    const float footer_height = candidates_height + error_height +
                                ImGui::GetFrameHeightWithSpacing() +
                                style.ItemSpacing.y;
    if (ImGui::BeginChild("##debug_history", ImVec2(0, -footer_height),
                          ImGuiChildFlags_Borders,
                          ImGuiWindowFlags_HorizontalScrollbar)) {
        for (const auto& record : m_history) {
            if (record.m_valid) {
                ImGui::TextUnformatted(record.m_command.c_str());
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                                   "Invalid Command: %s",
                                   record.m_command.c_str());
            }
        }
        if (m_scroll_to_bottom) {
            ImGui::SetScrollHereY(1.0f);
            m_scroll_to_bottom = false;
        }
    }
    ImGui::EndChild();

    ImGui::Separator();

    if (!m_candidates.empty()) {
        const int idx = m_selected < 0 ? 0 : m_selected;
        int clicked = -1;

        ImGuiChildFlags child_flags = ImGuiChildFlags_Borders;
        if (!candidates_scrollable) {
            child_flags |= ImGuiChildFlags_AutoResizeY;
        }
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        if (ImGui::BeginChild(
                "##debug_candidates",
                ImVec2(0, candidates_scrollable ? candidates_height : 0.0f),
                child_flags, ImGuiWindowFlags_HorizontalScrollbar)) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            const float content_width = ImGui::GetContentRegionAvail().x;
            const ImVec2 text_pad = style.FramePadding;

            for (int i = 0; i < static_cast<int>(m_candidates.size()); ++i) {
                ImGui::PushID(i);
                const ImVec2 pos = ImGui::GetCursorScreenPos();
                if (ImGui::InvisibleButton("##cand",
                                           ImVec2(content_width, row_height))) {
                    clicked = i;
                }
                const ImVec2 item_min = ImGui::GetItemRectMin();
                const ImVec2 item_max = ImGui::GetItemRectMax();
                const ImVec2 text_pos(pos.x + text_pad.x,
                                      pos.y + text_pad.y);

                if (i == idx) {
                    draw_list->AddRect(item_min, item_max,
                                       IM_COL32(255, 255, 0, 255));
                    draw_list->AddText(text_pos, IM_COL32(255, 255, 0, 255),
                                       m_candidates[i].c_str());
                    if (m_scroll_selected != idx) {
                        ImGui::SetScrollHereY(0.5f);
                        m_scroll_selected = idx;
                    }
                } else {
                    draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255),
                                       m_candidates[i].c_str());
                }
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        if (clicked >= 0) {
            m_pending_fill_text = BuildFillText(m_input, m_candidates[clicked]);
            m_pending_fill = true;
            m_last_input = m_pending_fill_text;
            m_candidates.clear();
            m_selected = -1;
            ImGui::SetKeyboardFocusHere();
        }
    }

    bool do_fill = false;
    bool do_execute = false;

    const ImGuiInputTextFlags input_flags =
        ImGuiInputTextFlags_EnterReturnsTrue |
        ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackAlways;

    if (ImGui::IsWindowAppearing() || m_force_input_focus) {
        ImGui::SetKeyboardFocusHere();
        m_force_input_focus = false;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputText("##debug_input", m_input, sizeof(m_input), input_flags,
                         &DebugPanel::inputTextCallback, this)) {
        if (!m_candidates.empty()) {
            do_fill = true;
        } else if (m_input[0] != '\0') {
            do_execute = true;
        }
    }

    const bool input_focused = ImGui::IsItemFocused();

    ImGuiIO& io = ImGui::GetIO();
    if (input_focused && io.KeyCtrl && !m_candidates.empty()) {
        if (ImGui::IsKeyPressed(ImGuiKey_P, true)) {
            moveSelection(-1);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_N, true)) {
            moveSelection(1);
        }
    }

    if (!m_candidates.empty() && ImGui::IsKeyPressed(ImGuiKey_Tab, true)) {
        moveSelection(io.KeyShift ? -1 : 1);
        m_force_input_focus = true;
    }

    if (do_fill) {
        const int idx = m_selected < 0 ? 0 : m_selected;
        m_pending_fill_text = BuildFillText(m_input, m_candidates[idx]);
        m_pending_fill = true;
        m_last_input = m_pending_fill_text;
        m_candidates.clear();
        m_selected = -1;
        ImGui::SetKeyboardFocusHere(-1);
    } else if (do_execute) {
        if (TrySubmitCommand(m_input)) {
            m_input[0] = '\0';
            m_last_input.clear();
            m_candidates.clear();
            m_selected = -1;
            m_history_cursor = -1;
        }
        ImGui::SetKeyboardFocusHere(-1);
    }

    if (!m_error_msg.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s",
                           m_error_msg.c_str());
    }

    ImGui::End();
}

void RegisterHFSMDebugCommands(DebugPanel& panel,
                               ClientHFSMDebugger& debugger) {
    panel.RegisterCmd("hfsm.toggle_visible", &debugger,
                      &ClientHFSMDebugger::ToggleVisible);
}
