#pragma once

#include "lua.h"
#include "lualib.h"
#include "engine/asset_manager_interface.hpp"
#include "engine/entity.hpp"
#include "engine/manager.hpp"
#include "engine/timer.hpp"

/** 从资产读入的 Luau 脚本源码数据 */
class ScriptBinaryData {
public:
    explicit ScriptBinaryData(const Path& path);
    ~ScriptBinaryData();

    /** 脚本源码内容（文件原始字节） */
    const std::vector<char>& GetContent() const;
    /** 用于 luau_load 的 chunk 名（由路径派生） */
    const std::string& GetModuleName() const;
    /** 用于日志/识别的脚本名（由文件名派生，兼容旧接口） */
    const std::string& GetClassName() const;

private:
    std::vector<char> m_content;
    std::string m_module_name;
    std::string m_class_name;
};

using ScriptBinaryDataHandle = Handle<ScriptBinaryData>;

/** 管理 Luau 脚本资产与全局 VM */
class ScriptBinaryDataManager : public AssetManagerBase<ScriptBinaryData> {
public:
    ScriptBinaryDataManager();
    ~ScriptBinaryDataManager();

    ScriptBinaryDataHandle Load(const Path& filename, bool force = false) override;
    /** 获取底层 Luau VM，供 Script 执行与绑定使用 */
    lua_State* GetUnderlyingVM();

private:
    lua_State* m_L{};

    void bindModule();
};

/** 挂到实体上的 Luau 脚本实例（对应一份脚本资产的一个运行实例） */
class Script {
public:
    Script(Entity entity, ScriptBinaryDataHandle handle);
    ~Script();

    void Update();
    void Render();

    /** Luau 中为脚本表在 LUA_REGISTRYINDEX 上的引用，供绑定/扩展使用 */
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
