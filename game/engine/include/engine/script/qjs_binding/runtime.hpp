#pragma once
#include "engine/script/qjs_binding/class_id.hpp"

// forward declare
struct JSRuntime;

class QJSClassFactory;
class QJSContext;

class QJSRuntime {
public:
    QJSRuntime();
    ~QJSRuntime();

    operator JSRuntime*() const;

    const QJSContext& GetContext() const;
    QJSContext& GetContext();
    QJSClassFactory& GetClassFactory();
    QJSClassIDManager& GetClassIDManager();
    QJSClassIDFamilyManager& GetClassIDFamilyManager();

private:
    JSRuntime* m_runtime{};
    std::unique_ptr<QJSClassFactory> m_class_factory;
    std::unique_ptr<QJSContext> m_context;
    std::unique_ptr<QJSClassIDManager> m_id_manager;
    std::unique_ptr<QJSClassIDFamilyManager> m_id_family_manager;
};