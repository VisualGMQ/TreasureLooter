#pragma once

#include "lua.h"
#include "lualib.h"
#include "engine/asset_manager_interface.hpp"
#include "engine/entity.hpp"
#include "engine/manager.hpp"
#include "engine/timer.hpp"

class ScriptBinaryData {
public:
    explicit ScriptBinaryData(const Path& path);
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
    lua_State* GetUnderlyingVM();

private:
    lua_State* m_L{};

    void bindModule();
};

class Script {
public:
    Script(Entity entity, ScriptBinaryDataHandle handle);
    ~Script();

    void Update();
    void Render();

    int GetScriptTableRef() const { return m_table_ref; }
    lua_State* GetVM() const { return m_L; }

private:
    lua_State* m_L{};
    int m_table_ref{LUA_NOREF};
    Entity m_entity{};

    bool m_inited = false;

    void callMethodNoArg(const char* method);
    void callMethodWithTime(const char* method, TimeType delta_time);
    void callMethodWithEntity(const char* method);
};

class ScriptComponentManager : public ComponentManager<Script> {
public:
    ScriptComponentManager();
    ~ScriptComponentManager();

    void Update();
    void Render();
};
