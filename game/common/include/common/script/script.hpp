#pragma once

#include "common/asset_manager_interface.hpp"
#include "common/context.hpp"
#include "common/entity.hpp"
#include "common/macros.hpp"
#include "common/manager.hpp"
#include "common/relationship.hpp"
#include "common/scene.hpp"
#include "common/script/script_require.hpp"
#include "common/timer.hpp"

#include "common/script/luabridge_include.hpp"

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

    void Initialize(const std::unordered_map<std::string, std::string>& lua_paths);

    ScriptBinaryDataHandle Load(const Path& filename,
                                bool force = false) override;
    lua_State* GetUnderlyingVM();

    auto& GetRequireContext() { return m_require_context; }

    void BindModule(std::function<void(lua_State*)> bind_func);

private:
    lua_State* m_L{};

    LuauRequireContext m_require_context;
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
    Path m_filename;

    bool m_inited = false;

    void callMethodNoArg(const char* method);
    void callMethodWithTime(const char* method, TimeType delta_time);
    void callMethodWithEntity(const char* method);

    void checkAndPrintErrorResult(const luabridge::LuaResult&,
                                  std::string_view method);

    struct PrepareInfo {
        luabridge::LuaRef m_instance;
        luabridge::LuaRef m_fn;

        PrepareInfo() : m_instance{nullptr}, m_fn{nullptr} {}

        PrepareInfo(luabridge::LuaRef instance, luabridge::LuaRef fn)
            : m_instance{instance}, m_fn{fn} {}

        operator bool() const noexcept {
            return m_instance.isValid() && m_fn.isValid();
        }
    };

    PrepareInfo prepareFn(std::string_view method);
};

class ScriptComponentManager : public ComponentManager<Script> {
public:
    ScriptComponentManager();
    ~ScriptComponentManager();

    void Update();
    void Render();

private:
    void doUpdate(Entity);
    void doRender(Entity);
};
