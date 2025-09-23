#pragma once
#include "engine/path.hpp"

// quickjs forward declare
struct JSValue;
struct JSContext;

class QJSRuntime;
class QJSModule;

class QJSContext {
public:
    explicit QJSContext(QJSRuntime& runtime);
    ~QJSContext();

    JSValue Eval(const std::vector<char>& content, const Path& filename,
                 bool strict_mode) const;
    JSValue EvalBinary(const std::vector<char>& content) const;

    QJSModule& NewModule(const std::string& name);

    auto& GetModules() const { return m_modules; }

    QJSRuntime& GetRuntime() const;

    operator JSContext*() const;
    operator bool() const;

private:
    JSContext* m_context{};
    QJSRuntime& m_runtime;

    std::vector<std::unique_ptr<QJSModule>> m_modules;
};
