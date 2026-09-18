#pragma once

#include "common/asset_manager_interface.hpp"
#include "common/context.hpp"
#include "common/entity.hpp"
#include "common/macros.hpp"
#include "common/manager.hpp"
#include "common/relationship.hpp"
#include "common/scene.hpp"
#include "common/timer.hpp"

#include "common/script/luabridge_include.hpp"

#include <memory>
#include <optional>
#include <string_view>

class ScriptBinaryData {
public:
    explicit ScriptBinaryData(const Path& path);
    ~ScriptBinaryData();

    const std::vector<char>& GetContent() const;
    const std::string& GetClassName() const;

    const Path& GetPath() const { return m_path; }

private:
    std::vector<char> m_content;
    std::string m_class_name;
    Path m_path;
};

using ScriptBinaryDataHandle = Handle<ScriptBinaryData>;

class ScriptBinaryDataManager : public AssetManagerBase<ScriptBinaryData> {
public:
    ScriptBinaryDataManager();
    ~ScriptBinaryDataManager();

    void Initialize();

    ScriptBinaryDataHandle Load(const Path& filename,
                                bool force = false) override;
    lua_State* GetUnderlyingVM();

    void BindModule(std::function<void(lua_State*)> bind_func);

private:
    lua_State* m_L{};
};

class Script {
public:
    Script(LogicEntity entity, ScriptBinaryDataHandle handle);
    ~Script();

    void callMethodNoArg(const char* method);
    void callMethodWithTime(const char* method, TimeType delta_time);
    void callMethodWithEntity(const char* method);

    bool IsInited() const { return m_inited; }
    void MarkInited() { m_inited = true; }

    int GetScriptTableRef() const { return m_table_ref; }

    lua_State* GetVM() const { return m_L; }

private:
    lua_State* m_L{};
    int m_table_ref{LUA_NOREF};
    LogicEntity m_entity{};
    Path m_filename;

    bool m_inited = false;

    struct PrepareInfo {
        luabridge::LuaRef m_instance;
        luabridge::LuaRef m_fn;

        PrepareInfo(luabridge::LuaRef instance, luabridge::LuaRef fn)
            : m_instance{instance}, m_fn{fn} {}
    };

    std::optional<PrepareInfo> prepareFn(std::string_view method);
};

class ScriptComponentManager : public ComponentManager<Script> {
public:
    ScriptComponentManager();
    ~ScriptComponentManager();

    void Update();
    void Render();

private:
    void doUpdate(LogicEntity);
    void doRender(LogicEntity);
};
