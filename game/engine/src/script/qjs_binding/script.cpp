#include "engine/script/qjs_binding/script.hpp"

#include "engine/context.hpp"
#include "engine/storage.hpp"
#include "engine/script/qjs_binding/common.hpp"
#include "engine/script/qjs_binding/context.hpp"

QJSScript::QJSScript(const Path& path) {
    auto io = IOStream::CreateFromFile(path, IOMode::Read, true);
    auto content = io->Read();

    auto& manager = static_cast<QJSScriptManager&>(*CURRENT_CONTEXT.
        m_script_manager);
    m_context = &manager.GetRuntime().GetContext();
    m_value = m_context->Eval(content, path.string(), true);
}

QJSScript::QJSScript(const std::vector<char>& binary) {
    auto& manager = static_cast<QJSScriptManager&>(*CURRENT_CONTEXT.
        m_script_manager);
    m_context = &manager.GetRuntime().GetContext();
    m_value = m_context->EvalBinary(binary);
}

QJSScript::~QJSScript() {
    JS_FreeValue(*m_context, m_value);
}

void QJSScript::OnUpdate() {
    // TODO:
}

QJSScriptManager::QJSScriptManager() {
    // TODO: register script
    // RegisterQJSScript(m_runtime);
}

void QJSScriptManager::Eval(const std::vector<char>& code) {
    auto& context = m_runtime.GetContext();
    JSValue value = context.Eval(code, "", false);
    JS_FreeValue(context, value);
}

void QJSScriptManager::EvalBinary(const std::vector<char>& code) {
    auto& context = m_runtime.GetContext();
    JSValue value = context.EvalBinary(code);
    JS_FreeValue(context, value);
}

QJSRuntime& QJSScriptManager::GetRuntime() {
    return m_runtime;
}
