#include "client/debug_panel.hpp"
#include "client/context.hpp"
#include "client/input/input.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "imgui.h"

#define FTS_FUZZY_MATCH_IMPLEMENTATION
#include "fts_fuzzy_match.hpp"

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <utility>

void DebugPanel::RegisterCmd(const std::string& name, debug_command_type cmd) {
    if (m_commands.find(name) == m_commands.end()) {
        m_cmd_names.push_back(name);
    }
    m_commands[name] = cmd;
}

void DebugPanel::RegisterCmd(const std::string& name, luabridge::LuaRef cmd) {
    if (!cmd.isCallable()) {
        LOGE("[DebugPanel] RegisterCmd from luau '{}' is not callable", name);
        return;
    }

    if (m_commands.find(name) == m_commands.end()) {
        m_cmd_names.push_back(name);
    }
    m_commands[name] = [name, cmd](const std::vector<std::string>& args) {
        auto result = cmd(args);
        if (result.errorCode()) {
            LOGE("[DebugPanel] lua command '{}' error: {}", name,
                 result.errorMessage());
        }
    };
}

std::vector<std::string> DebugPanel::collectFuzzyMatches(
    const std::string& name) const {
    std::vector<std::pair<int, const std::string*>> scored;

    if (name.empty()) {
        for (const auto& cmd_name : m_cmd_names) {
            scored.emplace_back(0, &cmd_name);
        }
    } else {
        for (const auto& cmd_name : m_cmd_names) {
            int score = 0;
            if (fts::fuzzy_match(name.c_str(), cmd_name.c_str(), score)) {
                scored.emplace_back(score, &cmd_name);
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

    it->second(args);
}

void DebugPanel::refreshCandidates() {
    if (m_last_input == m_input) {
        return;
    }

    m_last_input = m_input;
    // Once a space is typed the user is entering arguments, so stop suggesting.
    if (m_input[0] == '\0' || m_last_input.find(' ') != std::string::npos) {
        m_candidates.clear();
    } else {
        m_candidates = collectFuzzyMatches(m_last_input);
    }
    m_selected = m_candidates.empty() ? -1 : 0;
    m_history_cursor = -1;
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
    const std::string cmd_name = tokens.empty() ? std::string() : tokens.front();
    const bool valid = m_commands.find(cmd_name) != m_commands.end();
    m_history.push_back({name, valid});
    m_scroll_to_bottom = true;

    if (valid) {
        const std::vector<std::string> args(tokens.begin() + 1, tokens.end());
        ExecuteCmd(cmd_name, args);
    }
}

int DebugPanel::inputTextCallback(ImGuiInputTextCallbackData* data) {
    auto* self = static_cast<DebugPanel*>(data->UserData);
    return self->handleInputCallback(data);
}

int DebugPanel::handleInputCallback(ImGuiInputTextCallbackData* data) {
    switch (data->EventFlag) {
        case ImGuiInputTextFlags_CallbackCompletion: {
            if (!m_candidates.empty()) {
                const int idx = m_selected < 0 ? 0 : m_selected;
                const std::string cand = m_candidates[idx];
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, cand.c_str());
                m_last_input = cand;
                m_candidates.clear();
                m_selected = -1;
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
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (!ImGui::Begin("##debug_panel", nullptr, window_flags)) {
        ImGui::End();
        return;
    }

    const float footer_height = ImGui::GetFrameHeightWithSpacing() * 2.0f +
                                ImGui::GetStyle().ItemSpacing.y;
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

    refreshCandidates();

    bool do_fill = false;
    bool do_execute = false;

    const ImGuiInputTextFlags input_flags =
        ImGuiInputTextFlags_EnterReturnsTrue |
        ImGuiInputTextFlags_CallbackCompletion |
        ImGuiInputTextFlags_CallbackHistory;

    if (ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere();
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

    if (do_fill) {
        const int idx = m_selected < 0 ? 0 : m_selected;
        const std::string cand = m_candidates[idx];
        std::snprintf(m_input, sizeof(m_input), "%s", cand.c_str());
        m_last_input = cand;
        m_candidates.clear();
        m_selected = -1;
        ImGui::SetKeyboardFocusHere(-1);
    } else if (do_execute) {
        submitCommand(m_input);
        m_input[0] = '\0';
        m_last_input.clear();
        m_candidates.clear();
        m_selected = -1;
        m_history_cursor = -1;
        ImGui::SetKeyboardFocusHere(-1);
    }

    if (!m_candidates.empty()) {
        const int idx = m_selected < 0 ? 0 : m_selected;
        int clicked = -1;
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##debug_candidates", m_candidates[idx].c_str(),
                              ImGuiComboFlags_NoArrowButton)) {
            for (int i = 0; i < static_cast<int>(m_candidates.size()); ++i) {
                const bool is_selected = (i == idx);
                if (ImGui::Selectable(m_candidates[i].c_str(), is_selected)) {
                    clicked = i;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (clicked >= 0) {
            const std::string cand = m_candidates[clicked];
            std::snprintf(m_input, sizeof(m_input), "%s", cand.c_str());
            m_last_input = cand;
            m_candidates.clear();
            m_selected = -1;
            ImGui::SetKeyboardFocusHere();
        }
    }

    ImGui::End();
}
