#pragma once

#include "angelscript.h"
#include "engine/asset_manager_interface.hpp"
#include "engine/manager.hpp"

class ScriptBinaryData {
public:
    ScriptBinaryData(const Path&, asIScriptEngine*);
    ~ScriptBinaryData();

    const std::vector<char>& GetContent() const;

private:
    std::vector<char> m_content;
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
    void buildSharedModule();
};

class Script {
public:
    Script(Entity entity, ScriptBinaryDataHandle);
    ~Script();

    void Update();

private:
    asIScriptContext* m_ctx{};
    asIScriptFunction* m_init_fn{};
    asIScriptFunction* m_update_fn{};
    asIScriptFunction* m_quit_fn{};
    asIScriptObject* m_class_instance{};

    bool m_inited = false;

    void callMethod(asIScriptFunction*);
};

class ScriptComponentManager : public ComponentManager<Script> {
public:
    ScriptComponentManager();
    ~ScriptComponentManager();

    void Update();
};
