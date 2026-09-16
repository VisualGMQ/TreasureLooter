#pragma once
#include <charconv>
#include <functional>
#include <string>
#include <system_error>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "common/macros.hpp"
#include "common/script/luabridge_include.hpp"
#include "schema/gameplay_config.hpp"
#include "spdlog/fmt/bundled/format.h"

struct ImGuiInputTextCallbackData;
class ClientHFSMDebugger;

template <typename... Args>
using debug_command_type = void(Args...);

template <typename Class, typename... Args>
using class_debug_command_type = void (Class::*)(Args...);

using debug_panel_param_hint_fn = std::function<std::vector<std::string>(void)>;

namespace internal {

struct ParamConvertError {
    explicit ParamConvertError(const std::string& error_mg);
    ParamConvertError() = default;

    operator bool() const;
    [[nodiscard]] std::string_view GetErrMsg() const;

private:
    bool m_error = false;
    std::string m_err_msg;
};

ParamConvertError ConvertParam(std::string_view text, LogicEntity& out_value);
ParamConvertError ConvertParam(std::string_view text, bool& out_value);
ParamConvertError ConvertParam(std::string_view text, std::string& out_value);

template <typename T, typename = std::enable_if_t<std::is_integral_v<T> ||
                                                  std::is_floating_point_v<T>>>
ParamConvertError ConvertParam(std::string_view text, T& out_value) {
    auto result =
        std::from_chars(text.data(), text.data() + text.length(), out_value);
    ParamConvertError error(fmt::format("can't convert {} to number", text));
    TL_RETURN_VALUE_IF_FALSE(result.ec == std::errc{}, error);
    return {};
}

template <typename T>
T convertParamWithError(std::string_view text,
                        std::vector<ParamConvertError>& errors) {
    T value;
    if (auto err = ConvertParam(text, value)) {
        errors.push_back(err);
    }
    return value;
}

template <typename... Args, size_t... Idx>
std::tuple<Args...> convertAllParams(
    const std::vector<std::string>& args_strings,
    std::vector<ParamConvertError>& errors, std::index_sequence<Idx...>) {
    return std::make_tuple<Args...>(
        convertParamWithError<Args>(args_strings[Idx], errors)...);
}

template <typename... Args>
void CallDebugCMD(debug_command_type<Args...>* fn,
                  const std::vector<std::string>& arg_strings) {
    TL_RETURN_IF_NULL(fn);
    TL_RETURN_IF_FALSE(arg_strings.size() != sizeof...(Args));

    if constexpr (sizeof...(Args) == 0) {
        std::invoke(fn);
    } else {
        std::vector<ParamConvertError> errors;
        std::tuple<Args...> args = convertAllParams(
            arg_strings, errors, std::make_index_sequence<sizeof...(Args)>{});
        std::apply(fn, args);
    }
}

template <typename Class, typename... Args>
void CallDebugCMD(class_debug_command_type<Class, Args...> fn,
                  Class* class_instance,
                  const std::vector<std::string>& arg_strings) {
    TL_RETURN_IF_NULL(fn);
    TL_RETURN_IF_FALSE_WITH_LOG(arg_strings.size() >= sizeof...(Args), LOGE,
                                "[DebugPanel]: param size not satisfied");

    if constexpr (sizeof...(Args) == 0) {
        std::invoke(fn, class_instance);
    } else {
        std::vector<ParamConvertError> errors;
        std::tuple<Args...> args = convertAllParams<Args...>(
            arg_strings, errors, std::make_index_sequence<sizeof...(Args)>{});
        std::tuple<Class*, Args...> args_with_class =
            std::tuple_cat(std::tuple<Class*>(class_instance), args);
        std::apply(fn, args_with_class);
    }
}

template <typename T>
std::vector<std::string> GetDebugPanelParamHint();

template <>
std::vector<std::string> GetDebugPanelParamHint<LogicEntity>();
template <>
std::vector<std::string> GetDebugPanelParamHint<bool>();
template <>
std::vector<std::string> GetDebugPanelParamHint<std::string>();
template <>
std::vector<std::string> GetDebugPanelParamHint<DID>();
template <>
std::vector<std::string> GetDebugPanelParamHint<float>();
template <>
std::vector<std::string> GetDebugPanelParamHint<double>();

template <typename... Args>
std::vector<debug_panel_param_hint_fn> GetAllHints() {
    std::vector<debug_panel_param_hint_fn> results;
    (results.emplace_back(&GetDebugPanelParamHint<Args>), ...);
    return results;
}

}  // namespace internal

class DebugPanel {
public:
    template <typename... Args>
    void RegisterCmd(const std::string& name,
                     debug_command_type<Args...>* cmd) {
        auto fn = [=](const std::vector<std::string>& args) {
            internal::CallDebugCMD<Args...>(cmd, args);
        };

        if (m_commands.find(name) == m_commands.end()) {
            m_cmd_names.push_back(name);
        }

        DebugCmd debug_cmd;
        debug_cmd.m_fn = fn;
        debug_cmd.m_param_hints = internal::GetAllHints<Args...>();
        m_commands.try_emplace(name, debug_cmd);
    }

    template <typename Class, typename... Args>
    void RegisterCmd(const std::string& name, Class* class_instance,
                     class_debug_command_type<Class, Args...> cmd) {
        auto fn = [=](const std::vector<std::string>& args) {
            internal::CallDebugCMD<Class, Args...>(cmd, class_instance, args);
        };

        if (m_commands.find(name) == m_commands.end()) {
            m_cmd_names.push_back(name);
        }
        DebugCmd debug_cmd;
        debug_cmd.m_fn = fn;
        debug_cmd.m_param_hints = internal::GetAllHints<Args...>();
        m_commands.try_emplace(name, debug_cmd);
    }

    void RegisterCmd(const std::string& name, luabridge::LuaRef cmd,
                     const std::vector<luabridge::LuaRef>& param_hints = {});
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

    struct DebugCmd {
        std::function<void(const std::vector<std::string>&)> m_fn;
        std::vector<debug_panel_param_hint_fn> m_param_hints;
    };

    [[nodiscard]] std::vector<std::string> collectFuzzyMatches(
        const std::string& name) const;
    [[nodiscard]] std::vector<std::string> collectFuzzyMatchesFrom(
        const std::string& pattern,
        const std::vector<std::string>& items) const;
    [[nodiscard]] std::vector<std::string> collectCandidates(
        const std::string& input) const;
    void refreshCandidates();
    void moveSelection(int dir);
    void submitCommand(const std::string& name);
    bool TrySubmitCommand(const std::string& input);
    std::string BuildFillText(const std::string& input,
                              const std::string& candidate);
    static std::vector<std::string> splitTokens(const std::string& input);
    int handleInputCallback(ImGuiInputTextCallbackData* data);
    static int inputTextCallback(ImGuiInputTextCallbackData* data);

    std::unordered_map<std::string, DebugCmd> m_commands;
    std::vector<std::string> m_cmd_names;

    std::vector<HistoryRecord> m_history;

    char m_input[256] = {};
    std::string m_last_input;
    std::string m_fuzzy_result;
    std::string m_error_msg;
    std::vector<std::string> m_candidates;
    int m_selected = -1;
    int m_scroll_selected = -1;
    int m_history_cursor = -1;
    bool m_scroll_to_bottom = false;
    bool m_force_input_focus = false;
    bool m_pending_fill = false;
    std::string m_pending_fill_text;
    bool m_visible = false;
};

void RegisterHFSMDebugCommands(DebugPanel& panel,
                               ClientHFSMDebugger& debugger);
