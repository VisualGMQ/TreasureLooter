#include "engine/script/script.hpp"
#include "engine/asset_manager.hpp"
#include "engine/log.hpp"
#include "engine/macros.hpp"
#include "engine/path.hpp"
#include "engine/script/script_binding.hpp"
#include "engine/script/script_event_registry.hpp"
#include "engine/script/script_require.hpp"
#include "engine/storage.hpp"
#include "engine/ui.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <type_traits>

#include "lua.h"
#include "luacode.h"
#include "lualib.h"

static std::string pathToClassName(const std::string& filename_str) {
    if (filename_str.empty()) return "Script";
    std::filesystem::path p(filename_str);
    std::string class_name = p.filename().replace_extension("").string();
    if (class_name.empty()) return "Script";
    class_name[0] = static_cast<char>(
        std::toupper(static_cast<unsigned char>(class_name[0])));
    for (size_t i = 1; i < class_name.size();) {
        if (class_name[i] == '-' || class_name[i] == '_') {
            if (i + 1 < class_name.size())
                class_name[i + 1] = static_cast<char>(std::toupper(
                    static_cast<unsigned char>(class_name[i + 1])));
            class_name.erase(class_name.begin() +
                             static_cast<std::ptrdiff_t>(i));
        } else {
            ++i;
        }
    }
    return class_name;
}

// -----------------------------------------------------------------------------
// ScriptBinaryData
// -----------------------------------------------------------------------------

ScriptBinaryData::ScriptBinaryData(const Path& path) : m_path(path) {
    auto io = IOStream::CreateFromFile(path, IOMode::Read, true);
    m_content = io->Read();

    TL_RETURN_IF_FALSE_WITH_LOG(!m_content.empty(), LOGE,
                                "read script {} failed", path);

    std::string path_str = path.string();
    m_class_name = pathToClassName(path_str);
}

ScriptBinaryData::~ScriptBinaryData() = default;

const std::vector<char>& ScriptBinaryData::GetContent() const {
    return m_content;
}

const std::string& ScriptBinaryData::GetClassName() const {
    return m_class_name;
}

// -----------------------------------------------------------------------------
// ScriptBinaryDataManager
// -----------------------------------------------------------------------------

ScriptBinaryDataManager::ScriptBinaryDataManager() {}

void ScriptBinaryDataManager::bindModule() {
    BindTLModule(m_L);
}

ScriptBinaryDataManager::~ScriptBinaryDataManager() {
    if (m_L) lua_close(m_L);
}

void ScriptBinaryDataManager::Initialize(const GameConfig& game_config) {
    for (auto& [name, path] : game_config.m_lua_paths) {
        m_require_context.RegisterAliasPath(name, path);
    }

    m_L = luaL_newstate();
    if (!m_L) {
        LOGE("Luau VM init failed!");
        return;
    }
    luaL_openlibs(m_L);
    m_require_context.InitModuleRegisterTable(m_L);
    bindModule();
    m_require_context.BindRequire(m_L);
}

ScriptBinaryDataHandle ScriptBinaryDataManager::Load(const Path& filename,
                                                     bool force) {
    if (auto it = Find(filename); it && !force) {
        return it;
    }
    return store(&filename, UUID::CreateV4(),
                 std::make_unique<ScriptBinaryData>(filename));
}

lua_State* ScriptBinaryDataManager::GetUnderlyingVM() {
    return m_L;
}

// -----------------------------------------------------------------------------
// ScriptComponentManager
// -----------------------------------------------------------------------------

ScriptComponentManager::ScriptComponentManager() = default;

ScriptComponentManager::~ScriptComponentManager() = default;

void ScriptComponentManager::Update() {
    auto level = CURRENT_CONTEXT.m_level_manager->GetCurrentLevel();
    TL_RETURN_IF_FALSE(level);

    doUpdate(level->GetRootEntity());
    doUpdate(level->GetUIRootEntity());
}

void ScriptComponentManager::Render() {
    auto level = CURRENT_CONTEXT.m_level_manager->GetCurrentLevel();
    TL_RETURN_IF_FALSE(level);

    doRender(level->GetRootEntity());
    doRender(level->GetUIRootEntity());
}

void ScriptComponentManager::doUpdate(Entity entity) {
    if (auto it = m_components.find(entity);
        it != m_components.end() && it->second.m_enable) {
        it->second.m_component->Update();
    }

    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    TL_RETURN_IF_FALSE(relationship);
    for (auto child : relationship->m_children) {
        doUpdate(child);
    }
}

void ScriptComponentManager::doRender(Entity entity) {
    if (auto it = m_components.find(entity);
        it != m_components.end() && it->second.m_enable) {
        it->second.m_component->Render();
    }

    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    TL_RETURN_IF_FALSE(relationship);
    for (auto child : relationship->m_children) {
        doRender(child);
    }
}

// -----------------------------------------------------------------------------
// Script
// -----------------------------------------------------------------------------

Script::Script(Entity entity, ScriptBinaryDataHandle handle)
    : m_entity(entity) {
    TL_RETURN_IF_FALSE(handle);

    m_L = CURRENT_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>()
              .GetUnderlyingVM();
    TL_RETURN_IF_NULL_WITH_LOG(m_L, LOGE, "[Luau]: VM is null");

    std::string script_path = handle->GetPath().string();

    bool from_cache = LuauRequireContext::GetCached(m_L, script_path);
    if (!from_cache) {
        const std::vector<char>& source = handle->GetContent();
        TL_RETURN_IF_FALSE_WITH_LOG(!source.empty(), LOGE,
                                    "[Luau]: script content empty");

        size_t bytecode_size = 0;
        char* bytecode =
            luau_compile(source.data(), source.size(), nullptr, &bytecode_size);
        TL_RETURN_IF_NULL_WITH_LOG(bytecode, LOGE, "[Luau]: compile failed");

        int load_result = luau_load(m_L, handle->GetClassName().c_str(),
                                    bytecode, bytecode_size, 0);
        free(bytecode);

        if (load_result != 0) {
            const char* err = lua_tostring(m_L, -1);
            LOGE("[Luau]: load failed: {}", err ? err : "unknown");
            lua_pop(m_L, 1);
            return;
        }

        int pcall_result = lua_pcall(m_L, 0, 1, 0);
        if (pcall_result != LUA_OK) {
            const char* err = lua_tostring(m_L, -1);
            LOGE("[Luau]: script run failed: {}", err ? err : "unknown");
            lua_pop(m_L, 1);
            return;
        }

        LuauRequireContext::SetCached(m_L, script_path);
    }

    if (!lua_istable(m_L, -1)) {
        LOGE("[Luau]: script must return a table: name {}, with OnInit & "
             "OnUpdate & OnRender & OnQuit",
             handle->GetClassName());
        lua_pop(m_L, 1);
        return;
    }

    luabridge::LuaRef class_table = luabridge::LuaRef::fromStack(m_L, -1);
    luabridge::LuaRef new_fn = class_table.rawget("new");
    const lua_Integer entity_val = static_cast<lua_Integer>(
        static_cast<std::underlying_type_t<Entity>>(m_entity));

    if (new_fn.isFunction()) {
        auto new_result = new_fn(entity_val);
        if (new_result && new_result.size() > 0) {
            luabridge::LuaRef instance = new_result[0];
            if (instance.isTable()) {
                instance.push(m_L);
                m_table_ref = lua_ref(m_L, -1);
                lua_pop(m_L, 2);  // instance + class_table
                TL_RETURN_IF_FALSE_WITH_LOG(
                    m_table_ref != LUA_NOREF, LOGE,
                    "[Luau]: failed to ref script instance");
                return;
            }
        }
        if (!new_result)
            LOGE("[Script]: script new() failed: {}",
                 new_result.errorMessage());
    } else {
        LOGE("[Script]: module {} must has new(Entity) function",
             handle->GetClassName());
    }
}

void Script::Update() {
    TL_RETURN_IF_FALSE(m_L && m_table_ref != LUA_NOREF);

    if (!m_inited) {
        callMethodWithEntity("OnInit");
        m_inited = true;
    }

    callMethodWithTime("OnUpdate", CURRENT_CONTEXT.m_time->GetElapseTime());
}

void Script::Render() {
    TL_RETURN_IF_FALSE(m_L && m_inited && m_table_ref != LUA_NOREF);
    callMethodNoArg("OnRender");
}

void Script::SubscribeEvent(std::string_view event_name) {
    if (auto id = ScriptEventRegistry::Lookup(event_name);
        id != kInvalidTypeIndex) {
        m_subscribed_events.insert(id);
    } else {
        LOGE("[Script]: SubscribeEvent unknown name: {}", event_name);
    }
}

void ScriptComponentManager::SubscribeEvent(Entity entity,
                                            const std::string& event_name) {
    if (Script* script = Get(entity)) {
        script->SubscribeEvent(event_name);
    }
}

void Script::callMethodNoArg(const char* method) {
    auto prepare = prepareFn(method);
    TL_RETURN_IF_FALSE(prepare);

    auto result = prepare.m_fn(prepare.m_instance);
    checkAndPrintErrorResult(result, method);
}

void Script::callMethodWithTime(const char* method, TimeType delta_time) {
    auto prepare = prepareFn(method);
    TL_RETURN_IF_FALSE(prepare);

    auto result =
        prepare.m_fn(prepare.m_instance, static_cast<lua_Number>(delta_time));
    checkAndPrintErrorResult(result, method);
}

void Script::callMethodWithEntity(const char* method) {
    auto prepare = prepareFn(method);
    TL_RETURN_IF_FALSE(prepare);

    lua_Integer entity_val = static_cast<lua_Integer>(
        static_cast<std::underlying_type_t<Entity>>(m_entity));
    auto result = prepare.m_fn(prepare.m_instance, entity_val);
    checkAndPrintErrorResult(result, method);
}

void Script::checkAndPrintErrorResult(const luabridge::LuaResult& result,
                                      std::string_view method) {
    TL_RETURN_IF_TRUE(result);

    std::string msg = result.errorMessage();
    const char* fallback = "(no detailed Lua error text; likely wrong "
                           "call signature or non-string error object)";
    bool success = std::any_of(msg.begin(), msg.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    });
    LOGE("[Luau] {}: {}", method, success ? msg : fallback);
}

Script::PrepareInfo Script::prepareFn(std::string_view method) {
    lua_getref(m_L, m_table_ref);
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        return {};
    }
    lua_pushstring(m_L, method.data());
    lua_gettable(m_L, -2);
    luabridge::LuaRef fn = luabridge::LuaRef::fromStack(m_L, -1);
    luabridge::LuaRef instance = luabridge::LuaRef::fromStack(m_L, -2);
    lua_pop(m_L, 2);

    if (!fn.isCallable()) return {};

    return PrepareInfo{instance, fn};
}

Script::~Script() {
    if (m_inited) callMethodNoArg("OnQuit");

    if (m_L && m_table_ref != LUA_NOREF) {
        lua_unref(m_L, m_table_ref);
        m_table_ref = LUA_NOREF;
    }
}
