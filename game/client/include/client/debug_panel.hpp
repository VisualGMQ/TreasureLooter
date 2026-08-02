#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/script/luabridge_include.hpp"

struct ImGuiInputTextCallbackData;

class DebugPanel {
public:
    using debug_command_type = std::function<void(const std::vector<std::string>&)>;

    void RegisterCmd(const std::string& name, debug_command_type cmd);
    void RegisterCmd(const std::string& name, luabridge::LuaRef cmd);
    const std::string& FindCmdFuzzy(const std::string& name);
    void ExecuteCmd(const std::string& name,
                    const std::vector<std::string>& args = {});

    void Update();
    void Render();

private:
    struct HistoryRecord {
        std::string m_command;
        bool m_valid;
    };

    [[nodiscard]] std::vector<std::string> collectFuzzyMatches(const std::string& name) const;
    void refreshCandidates();
    void moveSelection(int dir);
    void submitCommand(const std::string& name);
    static std::vector<std::string> splitTokens(const std::string& input);
    int handleInputCallback(ImGuiInputTextCallbackData* data);
    static int inputTextCallback(ImGuiInputTextCallbackData* data);

    std::unordered_map<std::string, debug_command_type> m_commands;
    std::vector<std::string> m_cmd_names;

    std::vector<HistoryRecord> m_history;

    char m_input[256] = {};
    std::string m_last_input;
    std::string m_fuzzy_result;
    std::vector<std::string> m_candidates;
    int m_selected = -1;
    int m_history_cursor = -1;
    bool m_scroll_to_bottom = false;
    bool m_visible = false;
};
