#pragma once
#include "engine/script/qjs_binding/context.hpp"
#include "engine/script/qjs_binding/runtime.hpp"
#include "engine/script/script.hpp"

class QJSScript: public Script {
public:
    QJSScript(const Path&);
    QJSScript(const std::vector<char>&);
    ~QJSScript() override;
    
    void OnInit() override;
    void OnEnter() override;
    void OnUpdate() override;
    void OnQuit() override;
    void OnDestroy() override;

private:
    QJSContext* m_context{};
    JSValue m_value;
};

class QJSScriptManager: public ScriptManager {
public:
    QJSScriptManager();
    void Eval(const std::vector<char>& code);
    void EvalBinary(const std::vector<char>&);

    QJSRuntime& GetRuntime();
     
private:
    QJSRuntime m_runtime;
};
