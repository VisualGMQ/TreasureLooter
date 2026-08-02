#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/script/luabridge_include.hpp"

struct ImGuiInputTextCallbackData;

class IDebugPanel {
public:
    using debug_command_type =
        std::function<void(const std::vector<std::string>&)>;

    virtual ~IDebugPanel() = default;

    virtual void RegisterCmd(const std::string& name,
                             debug_command_type cmd) = 0;
    virtual void RegisterCmd(const std::string& name,
                             luabridge::LuaRef cmd) = 0;
    virtual const std::string& FindCmdFuzzy(const std::string& name) = 0;
    virtual void ExecuteCmd(const std::string& name,
                            const std::vector<std::string>& args = {}) = 0;

    virtual void Update() = 0;
    virtual void Render() = 0;
};

class TrivialDebugPanel : public IDebugPanel {
public:
    void RegisterCmd(const std::string& name, debug_command_type cmd) override {
    }

    void RegisterCmd(const std::string& name, luabridge::LuaRef cmd) override {}

    const std::string& FindCmdFuzzy(const std::string& name) override {
        static std::string null;
        return null;
    }

    void ExecuteCmd(const std::string& name,
                    const std::vector<std::string>& args) override {}

    void Update() override {}

    void Render() override {}
};

class DebugPanel : public IDebugPanel {
public:
    void RegisterCmd(const std::string& name, debug_command_type cmd) override;
    void RegisterCmd(const std::string& name, luabridge::LuaRef cmd) override;
    const std::string& FindCmdFuzzy(const std::string& name) override;
    void ExecuteCmd(const std::string& name,
                    const std::vector<std::string>& args = {}) override;

    void Update() override;
    void Render() override;

private:
    struct HistoryRecord {
        std::string m_command;
        bool m_valid;
    };

    [[nodiscard]] std::vector<std::string> collectFuzzyMatches(
        const std::string& name) const;
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
