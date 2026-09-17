#include "common/script/script.hpp"
#include "common/asset_manager.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/path.hpp"
#include "common/profile.hpp"
#include "common/script/script_binding.hpp"
#include "common/storage.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <type_traits>

#include "common/script/luabridge_include.hpp"

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

ScriptBinaryDataManager::~ScriptBinaryDataManager() {
    if (m_L) lua_close(m_L);
}

namespace {

// `require` resolves modules through Lua's own file searcher, which reads files
// with the C standard library. That works on PC (the game scripts are plain
// files under the working directory) but not on Android, where the scripts are
// packed inside the APK and can only be read through SDL. This searcher reads
// the candidates with `IOStream` (i.e. `SDL_IOFromFile`), the same way the
// engine loads its entry scripts, so `require("client.world")` resolves on
// every platform. It is inserted ahead of the stock Lua file searcher, which
// stays in the list together with `package.path` as a fallback.
int EngineModuleSearcher(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    bool load_failed = false;

    // Scoped so that every C++ string is destroyed before the `lua_error`
    // below: raising unwinds with longjmp and would skip their destructors.
    {
        std::string module = name;
        std::replace(module.begin(), module.end(), '.', '/');

        const std::string candidates[] = {
            "scripts/" + module + ".lua",
            "scripts/" + module + "/init.lua",
        };

        std::string tried;
        for (const std::string& candidate : candidates) {
            if (!IOStream::Exists(candidate)) {
                if (!tried.empty()) {
                    tried += "\n\t";
                }
                tried += "no file '" + candidate + "'";
                continue;
            }

            auto stream =
                IOStream::CreateFromFile(candidate, IOMode::Read, true);
            if (!stream || !*stream) {
                continue;
            }

            const std::vector<char> content = stream->Read();
            const char* data = content.empty() ? "" : content.data();
            // The `@` prefix marks the chunk name as a file path, so Lua
            // reports errors and tracebacks as `scripts/xxx.lua:line`.
            const std::string chunk_name = "@" + candidate;
            if (luaL_loadbuffer(L, data, content.size(),
                                chunk_name.c_str()) != LUA_OK) {
                // The message is pushed while the C++ locals are still alive
                // and raised after they went out of scope.
                const char* reason = lua_tostring(L, -1);
                std::string message =
                    "error loading module '" + std::string(name) +
                    "' from file '" + candidate +
                    "':\n\t" + (reason ? reason : "unknown error");
                lua_pushlstring(L, message.data(), message.size());
                load_failed = true;
                break;
            }

            lua_pushstring(L, candidate.c_str());
            // chunk, plus the resolved path passed as its 2nd argument
            return 2;
        }

        if (!load_failed) {
            lua_pushfstring(L, "no module '%s' in engine scripts\n\t%s",
                            name, tried.c_str());
        }
    }

    if (load_failed) {
        return lua_error(L);  // raises the message pushed above
    }
    return 1;
}

}  // namespace

void ScriptBinaryDataManager::Initialize() {
    m_L = luaL_newstate();
    if (!m_L) {
        LOGE("Lua VM init failed!");
        return;
    }
    luaL_openlibs(m_L);

    // `package.path` stays as the PC fallback (the scripts are plain files
    // under the working directory, i.e. the `game/` folder). On Android they
    // live inside the APK, where the stock file searcher cannot reach them, so
    // the engine searcher installed below is what actually resolves `require`.
    lua_getglobal(m_L, "package");  // package
    if (lua_istable(m_L, -1)) {
        lua_getfield(m_L, -1, "path");  // package, path
        const char* existing = lua_tostring(m_L, -1);
        std::string path = "./scripts/?.lua;./scripts/?/init.lua";
        if (existing && existing[0] != '\0') {
            path += ";";
            path += existing;
        }
        lua_pop(m_L, 1);  // package
        lua_pushlstring(m_L, path.data(), path.size());
        lua_setfield(m_L, -2, "path");  // package.path = path

        // Inserted ahead of the stock Lua file searcher (slot 2; Lua 5.5
        // builds the list as preload, Lua files, C libs, C root), so game
        // modules are resolved through the engine first while the stock
        // searcher and `package.path` keep working as a fallback.
        lua_getfield(m_L, -1, "searchers");  // package, searchers
        if (lua_istable(m_L, -1)) {
            const size_t searcher_count = lua_rawlen(m_L, -1);
            for (size_t i = searcher_count + 1; i > 2; i--) {
                lua_rawgeti(m_L, -1, static_cast<lua_Integer>(i - 1));
                lua_rawseti(m_L, -2, static_cast<lua_Integer>(i));
            }
            lua_pushcfunction(m_L, EngineModuleSearcher);
            lua_rawseti(m_L, -2, 2);  // searchers[2] = EngineModuleSearcher
        }
        lua_pop(m_L, 1);  // searchers
    }
    lua_pop(m_L, 1);  // package
}

void ScriptBinaryDataManager::BindModule(std::function<void(lua_State*)> bind_func) {
    bind_func(m_L);
}

ScriptBinaryDataHandle ScriptBinaryDataManager::Load(const Path& filename,
                                                     bool force) {
    if (auto it = Find(filename); it && !force) {
        return it;
    }
    return store(&filename, UUIDv4::CreateV4(),
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
    PROFILE_SECTION();

    auto level = COMMON_CONTEXT.m_scene_manager->GetCurrentScene();
    TL_RETURN_IF_FALSE(level);

    doUpdate(level->GetRootEntity());
    doUpdate(level->GetUIRootEntity());
}

void ScriptComponentManager::Render() {
    PROFILE_SECTION();

    auto level = COMMON_CONTEXT.m_scene_manager->GetCurrentScene();
    TL_RETURN_IF_FALSE(level);

    doRender(level->GetRootEntity());
    doRender(level->GetUIRootEntity());
}

void ScriptComponentManager::doUpdate(LogicEntity entity) {
    PROFILE_SECTION();

    if (auto it = m_components.find(entity);
        it != m_components.end() && it->second.m_enable) {
        auto script = it->second.m_component.get();
        if (!script->IsInited()) {
            script->callMethodWithEntity("OnInit");
            script->MarkInited();
        }
        script->callMethodWithTime("OnUpdate",
                                   COMMON_CONTEXT.m_time->GetElapseTime());
    }

    auto relationship = COMMON_CONTEXT.m_relationship_manager->Get(entity);
    TL_RETURN_IF_FALSE(relationship);
    for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
        doUpdate(relationship->Get(i));
    }
}

void ScriptComponentManager::doRender(LogicEntity entity) {
    PROFILE_SECTION();

    if (auto it = m_components.find(entity);
        it != m_components.end() && it->second.m_enable) {
        it->second.m_component->callMethodNoArg("OnRender");
    }

    auto relationship = COMMON_CONTEXT.m_relationship_manager->Get(entity);
    TL_RETURN_IF_FALSE(relationship);
    for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
        doRender(relationship->Get(i));
    }
}

// -----------------------------------------------------------------------------
// Script
// -----------------------------------------------------------------------------

Script::Script(LogicEntity entity, ScriptBinaryDataHandle handle)
    : m_entity(entity) {
    TL_RETURN_IF_FALSE(handle);

    m_L = COMMON_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>()
              .GetUnderlyingVM();
    TL_RETURN_IF_NULL_WITH_LOG(m_L, LOGE, "[Lua]: VM is null");

    std::string script_path = handle->GetPath().string();
    m_filename = script_path;

    const std::vector<char>& source = handle->GetContent();
    TL_RETURN_IF_FALSE_WITH_LOG(!source.empty(), LOGE,
                                "[Lua]: script {} content empty", m_filename);

    int load_result =
        luaL_loadbuffer(m_L, source.data(), source.size(), script_path.c_str());
    if (load_result != LUA_OK) {
        const char* err = lua_tostring(m_L, -1);
        LOGE("[Lua]: load {} failed: {}", m_filename, err ? err : "unknown");
        lua_pop(m_L, 1);
        return;
    }

    int pcall_result = lua_pcall(m_L, 0, 1, 0);
    if (pcall_result != LUA_OK) {
        const char* err = lua_tostring(m_L, -1);
        LOGE("[Lua]: script {} run failed: {}", m_filename,
             err ? err : "unknown");
        lua_pop(m_L, 1);
        return;
    }

    if (!lua_istable(m_L, -1)) {
        LOGE("[Lua]: script {} must return a table: name {}, with OnInit & "
             "OnUpdate & OnRender & OnQuit",
             m_filename,
             handle->GetClassName());
        lua_pop(m_L, 1);
        return;
    }

    luabridge::LuaRef class_table = luabridge::LuaRef::fromStack(m_L, -1);
    luabridge::LuaRef new_fn = class_table.rawget("new");
    const lua_Integer entity_val = static_cast<lua_Integer>(
        static_cast<std::underlying_type_t<LogicEntity>>(m_entity));

    if (new_fn.isFunction()) {
        new_fn.push(m_L);  // class_table, new_fn
        const bool has_entity = m_entity != null_entity;
        if (has_entity) {
            lua_pushinteger(m_L, entity_val);  // class_table, new_fn, entity
        }
        const int call_result = lua_pcall(m_L, has_entity ? 1 : 0, 1, 0);
        if (call_result == LUA_OK && lua_istable(m_L, -1)) {
            m_table_ref = luaL_ref(m_L, LUA_REGISTRYINDEX);  // pops instance
            lua_pop(m_L, 1);                                 // class_table
            TL_RETURN_IF_FALSE_WITH_LOG(
                m_table_ref != LUA_NOREF, LOGE,
                "[Lua]: failed to ref script instance");
            return;
        }
        const char* err =
            call_result == LUA_OK ? "new() must return a table"
                                  : lua_tostring(m_L, -1);
        LOGE("[Script]: script {} new() failed: {}", m_filename,
             err ? err : "unknown");
        lua_pop(m_L, 1);  // result or error
        lua_pop(m_L, 1);  // class_table
        return;
    }

    LOGE("[Script]: module {} must has new(LogicEntity) function",
         handle->GetClassName());
    lua_pop(m_L, 1);  // class_table
}

// Runs `fn(self, ...)` through lua_pcall so the full Lua error message is
// available (LuaBridge's call helpers only report a generic failure).
void Script::callMethodNoArg(const char* method) {
    auto prepare = prepareFn(method);
    if (!prepare) return;

    prepare->m_fn.push(m_L);
    prepare->m_instance.push(m_L);
    if (lua_pcall(m_L, 1, 0, 0) != LUA_OK) {
        const char* err = lua_tostring(m_L, -1);
        LOGE("[Lua] {} {}: {}", m_filename, method, err ? err : "unknown");
        lua_pop(m_L, 1);
    }
}

void Script::callMethodWithTime(const char* method, TimeType delta_time) {
    auto prepare = prepareFn(method);
    if (!prepare) return;

    prepare->m_fn.push(m_L);
    prepare->m_instance.push(m_L);
    lua_pushnumber(m_L, static_cast<lua_Number>(delta_time));
    if (lua_pcall(m_L, 2, 0, 0) != LUA_OK) {
        const char* err = lua_tostring(m_L, -1);
        LOGE("[Lua] {} {}: {}", m_filename, method, err ? err : "unknown");
        lua_pop(m_L, 1);
    }
}

void Script::callMethodWithEntity(const char* method) {
    auto prepare = prepareFn(method);
    if (!prepare) return;

    lua_Integer entity_val = static_cast<lua_Integer>(
        static_cast<std::underlying_type_t<LogicEntity>>(m_entity));
    prepare->m_fn.push(m_L);
    prepare->m_instance.push(m_L);
    lua_pushinteger(m_L, entity_val);
    if (lua_pcall(m_L, 2, 0, 0) != LUA_OK) {
        const char* err = lua_tostring(m_L, -1);
        LOGE("[Lua] {} {}: {}", m_filename, method, err ? err : "unknown");
        lua_pop(m_L, 1);
    }
}

std::optional<Script::PrepareInfo> Script::prepareFn(std::string_view method) {
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_table_ref);
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        return std::nullopt;
    }
    lua_pushlstring(m_L, method.data(), method.size());
    lua_gettable(m_L, -2);
    luabridge::LuaRef fn = luabridge::LuaRef::fromStack(m_L, -1);
    luabridge::LuaRef instance = luabridge::LuaRef::fromStack(m_L, -2);
    lua_pop(m_L, 2);

    if (!fn.isCallable()) return std::nullopt;

    return PrepareInfo{instance, fn};
}

Script::~Script() {
    if (m_inited) callMethodNoArg("OnQuit");

    if (m_L && m_table_ref != LUA_NOREF) {
        luaL_unref(m_L, LUA_REGISTRYINDEX, m_table_ref);
        m_table_ref = LUA_NOREF;
    }
}
