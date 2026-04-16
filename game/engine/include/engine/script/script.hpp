#pragma once

#include "engine/asset_manager_interface.hpp"
#include "engine/context.hpp"
#include "engine/entity.hpp"
#include "engine/macros.hpp"
#include "engine/manager.hpp"
#include "engine/relationship.hpp"
#include "engine/script/script_require.hpp"
#include "engine/timer.hpp"
#include "lua.h"

#include <string_view>
#include <unordered_set>

struct GameConfig;

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

    void Initialize(const GameConfig&);

    ScriptBinaryDataHandle Load(const Path& filename,
                                bool force = false) override;
    lua_State* GetUnderlyingVM();

    auto& GetRequireContext() { return m_require_context; }

private:
    lua_State* m_L{};

    LuauRequireContext m_require_context;

    void bindModule();
};

class Script {
public:
    Script(Entity entity, ScriptBinaryDataHandle handle);
    ~Script();

    void Update();
    void Render();

    void SubscribeEvent(std::string_view event_name);

    template <typename T>
    bool IsSubscribedToEventType() const {
        return m_subscribed_events.count(TypeIndexGenerator::Get<T>()) != 0;
    }

    template <typename T>
    void HandleEvent(const T& event, std::string_view event_name) {
        TL_RETURN_IF_FALSE(m_L && m_table_ref != LUA_NOREF && m_inited);
        TL_RETURN_IF_FALSE(IsSubscribedToEventType<T>());

        std::string method;
        method.reserve(event_name.size() + 2);
        method = "On";
        method.append(event_name);
        auto prepare = prepareFn(method);
        TL_RETURN_IF_FALSE(prepare);

        auto result = prepare.m_fn(prepare.m_instance, event);
        checkAndPrintErrorResult(result, method);
    }

    int GetScriptTableRef() const { return m_table_ref; }

    lua_State* GetVM() const { return m_L; }

private:
    lua_State* m_L{};
    int m_table_ref{LUA_NOREF};
    Entity m_entity{};
    std::unordered_set<TypeIndex> m_subscribed_events;

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

    void SubscribeEvent(Entity entity, const std::string& event_name);

    template <typename T>
    void HandleEvent(const T& event, std::string_view event_name) {
        auto level = CURRENT_CONTEXT.m_scene_manager->GetCurrentScene();
        TL_RETURN_IF_FALSE(level);

        doHandleEvent(level->GetRootEntity(), event, event_name);
    }

private:
    void doUpdate(Entity);
    void doRender(Entity);

    template <typename T>
    void doHandleEvent(Entity entity, const T& event,
                       std::string_view event_name) {
        if (auto it = m_components.find(entity); it != m_components.end()) {
            TL_RETURN_IF_FALSE(it->second.m_enable);

            if (it->second.m_component->IsSubscribedToEventType<T>()) {
                it->second.m_component->HandleEvent(event, event_name);
            }
        }

        auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
        TL_RETURN_IF_FALSE(relationship);
        for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
            doHandleEvent(relationship->Get(i), event, event_name);
        }
    }
};
