#pragma once

#include "angelscript.h"
#include "engine/asset_manager_interface.hpp"
#include "engine/manager.hpp"
#include "engine/timer.hpp"

class ScriptBinaryData {
public:
    ScriptBinaryData(const Path&, asIScriptEngine*);
    ~ScriptBinaryData();

    const std::vector<char>& GetContent() const;
    const std::string& GetClassName() const;

private:
    std::vector<char> m_content;
    std::string m_class_name;
};

using ScriptBinaryDataHandle = Handle<ScriptBinaryData>;

class ScriptBinaryDataManager : public AssetManagerBase<ScriptBinaryData> {
public:
    ScriptBinaryDataManager();
    ~ScriptBinaryDataManager();

    ScriptBinaryDataHandle Load(const Path& filename, bool force = false) override;
    asIScriptEngine* GetUnderlyingEngine();

private:
    asIScriptEngine* m_engine{};

    void bindModule();
};

class Script {
public:
    Script(Entity entity, ScriptBinaryDataHandle);
    ~Script();

    void Update();

    asIScriptObject* GetScriptObject() const { return m_class_instance; }

private:
    asIScriptContext* m_ctx{};
    asIScriptFunction* m_init_fn{};
    asIScriptFunction* m_update_fn{};
    asIScriptFunction* m_quit_fn{};
    asIScriptObject* m_class_instance{};

    bool m_inited = false;

    void callNoArgMethod(asIScriptFunction*);
    void callUpdateMethod(asIScriptFunction*, TimeType);
};

class ScriptComponentManager : public ComponentManager<Script> {
public:
    ScriptComponentManager();
    ~ScriptComponentManager();

    void Update();
};
