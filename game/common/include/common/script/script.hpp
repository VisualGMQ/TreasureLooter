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

#include <memory>
#include <string_view>

// Opaque holder for the optional Luau DAP debugger. Defined in script.cpp so
// that consumers of this header do not need to see the debugger dependency.
struct ScriptDebuggerHolder;

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

    bool PathToModuleName(const Path& file_path, std::string& out_modname) const {
        return m_require_context.PathToModuleName(file_path, out_modname);
    }

    void BindModule(std::function<void(lua_State*)> bind_func);

    // Starts the Luau DAP debugger listening on `port` (port <= 0 disables
    // it). Must be called after Initialize() and before any script runs.
    void EnableDebugger(int port);

    // Tells the debugger a chunk has been loaded so breakpoints and stack
    // traces can be resolved. Safe to call when the debugger is disabled.
    void OnLuaFileLoaded(lua_State* L, const std::string& path, bool is_entry);

    // Forwards a Lua runtime error to the debugger console if enabled.
    void OnLuaError(const std::string& msg, lua_State* L);

private:
    lua_State* m_L{};

    LuauRequireContext m_require_context;

    std::unique_ptr<ScriptDebuggerHolder> m_debugger;
};

class Script {
public:
    Script(Entity entity, ScriptBinaryDataHandle handle);
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
    Entity m_entity{};
    Path m_filename;

    bool m_inited = false;

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
