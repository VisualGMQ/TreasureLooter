#include "engine/script/script.hpp"
#include "engine/script/script_binding.hpp"
#include "engine/asset_manager.hpp"
#include "engine/context.hpp"
#include "engine/log.hpp"
#include "engine/macros.hpp"
#include "engine/storage.hpp"
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <type_traits>

#include "luacode.h"

static std::string pathToModuleName(const std::string& path_str) {
    std::string name = path_str;
    for (char& c : name) {
        if (c == '/' || c == '\\' || c == '.')
            c = '_';
    }
    if (name.empty())
        name = "module";
    return name;
}

static std::string pathToClassName(const std::string& filename_str) {
    if (filename_str.empty())
        return "Script";
    std::filesystem::path p(filename_str);
    std::string class_name = p.filename().replace_extension("").string();
    if (class_name.empty())
        return "Script";
    class_name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(class_name[0])));
    for (size_t i = 1; i < class_name.size();) {
        if (class_name[i] == '-' || class_name[i] == '_') {
            if (i + 1 < class_name.size())
                class_name[i + 1] = static_cast<char>(std::toupper(static_cast<unsigned char>(class_name[i + 1])));
            class_name.erase(class_name.begin() + static_cast<std::ptrdiff_t>(i));
        } else {
            ++i;
        }
    }
    return class_name;
}

// -----------------------------------------------------------------------------
// ScriptBinaryData
// -----------------------------------------------------------------------------

ScriptBinaryData::ScriptBinaryData(const Path& path) {
    auto io = IOStream::CreateFromFile(path, IOMode::Read, true);
    m_content = io->Read();

    TL_RETURN_IF_FALSE_WITH_LOG(!m_content.empty(), LOGE,
                                "read script {} failed", path);

    std::string path_str = path.string();
    m_module_name = pathToModuleName(path_str);
    m_class_name = pathToClassName(path_str);
}

ScriptBinaryData::~ScriptBinaryData() = default;

const std::vector<char>& ScriptBinaryData::GetContent() const {
    return m_content;
}

const std::string& ScriptBinaryData::GetModuleName() const {
    return m_module_name;
}

const std::string& ScriptBinaryData::GetClassName() const {
    return m_class_name;
}

// -----------------------------------------------------------------------------
// ScriptBinaryDataManager
// -----------------------------------------------------------------------------

ScriptBinaryDataManager::ScriptBinaryDataManager() {
    m_L = luaL_newstate();
    if (!m_L) {
        LOGE("Luau VM init failed!");
        return;
    }
    luaL_openlibs(m_L);
    bindModule();
}

void ScriptBinaryDataManager::bindModule() {
    BindTLModule(m_L);
}

ScriptBinaryDataManager::~ScriptBinaryDataManager() {
    if (m_L)
        lua_close(m_L);
}

ScriptBinaryDataHandle ScriptBinaryDataManager::Load(const Path& filename,
                                                     bool force) {
    if (auto it = Find(filename); it && !force) {
        LOGW("script binary data {} already loaded", filename);
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
    for (auto&& [entity, component] : m_components) {
        component.m_component->Update();
    }
}

void ScriptComponentManager::Render() {
    for (auto&& [entity, component] : m_components) {
        component.m_component->Render();
    }
}

// -----------------------------------------------------------------------------
// Script
// -----------------------------------------------------------------------------

Script::Script(Entity entity, ScriptBinaryDataHandle handle) : m_entity(entity) {
    TL_RETURN_IF_FALSE(handle);

    m_L = CURRENT_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>()
              .GetUnderlyingVM();
    TL_RETURN_IF_NULL_WITH_LOG(m_L, LOGE, "[Luau]: VM is null");

    const std::vector<char>& source = handle->GetContent();
    TL_RETURN_IF_FALSE_WITH_LOG(!source.empty(), LOGE,
                                "[Luau]: script content empty");

    size_t bytecode_size = 0;
    char* bytecode = luau_compile(source.data(), source.size(), nullptr,
                                  &bytecode_size);
    TL_RETURN_IF_NULL_WITH_LOG(bytecode, LOGE, "[Luau]: compile failed");

    const std::string& chunkname = handle->GetModuleName();
    int load_result =
        luau_load(m_L, chunkname.c_str(), bytecode, bytecode_size, 0);
    std::free(bytecode);

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

    if (!lua_istable(m_L, -1)) {
        LOGE("[Luau]: script must return a table (with OnInit/OnUpdate/OnRender/OnQuit)");
        lua_pop(m_L, 1);
        return;
    }

    m_table_ref = lua_ref(m_L, -1);
    lua_pop(m_L, 1);
    TL_RETURN_IF_FALSE_WITH_LOG(m_table_ref != LUA_NOREF, LOGE,
                                "[Luau]: failed to ref script table");
}

void Script::Update() {
    TL_RETURN_IF_FALSE(m_L && m_table_ref != LUA_NOREF);

    if (!m_inited) {
        callMethodWithEntity("OnInit");
        m_inited = true;
    }

    callMethodWithTime("OnUpdate",
                       CURRENT_CONTEXT.m_time->GetElapseTime());
}

void Script::Render() {
    TL_RETURN_IF_FALSE(m_L && m_inited && m_table_ref != LUA_NOREF);
    callMethodNoArg("OnRender");
}

void Script::callMethodNoArg(const char* method) {
    lua_getref(m_L, m_table_ref);
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        return;
    }
    lua_getfield(m_L, -1, method);
    if (!lua_isfunction(m_L, -1)) {
        lua_pop(m_L, 2);
        return;
    }
    lua_pushvalue(m_L, -2);
    int r = lua_pcall(m_L, 1, 0, 0);
    lua_pop(m_L, 1);
    if (r != LUA_OK) {
        const char* err = lua_tostring(m_L, -1);
        LOGE("[Luau] {}: {}", method, err ? err : "unknown");
        lua_pop(m_L, 1);
    }
}

void Script::callMethodWithTime(const char* method, TimeType delta_time) {
    lua_getref(m_L, m_table_ref);
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        return;
    }
    lua_getfield(m_L, -1, method);
    if (!lua_isfunction(m_L, -1)) {
        lua_pop(m_L, 2);
        return;
    }
    lua_pushvalue(m_L, -2);
    lua_pushnumber(m_L, static_cast<lua_Number>(delta_time));
    int r = lua_pcall(m_L, 2, 0, 0);
    lua_pop(m_L, 1);
    if (r != LUA_OK) {
        const char* err = lua_tostring(m_L, -1);
        LOGE("[Luau] {}: {}", method, err ? err : "unknown");
        lua_pop(m_L, 1);
    }
}

void Script::callMethodWithEntity(const char* method) {
    lua_getref(m_L, m_table_ref);
    if (!lua_istable(m_L, -1)) {
        lua_pop(m_L, 1);
        return;
    }
    lua_getfield(m_L, -1, method);
    if (!lua_isfunction(m_L, -1)) {
        lua_pop(m_L, 2);
        return;
    }
    lua_pushvalue(m_L, -2);
    lua_pushinteger(m_L, static_cast<lua_Integer>(static_cast<std::underlying_type_t<Entity>>(m_entity)));
    int r = lua_pcall(m_L, 2, 0, 0);
    lua_pop(m_L, 1);
    if (r != LUA_OK) {
        const char* err = lua_tostring(m_L, -1);
        LOGE("[Luau] {}: {}", method, err ? err : "unknown");
        lua_pop(m_L, 1);
    }
}

Script::~Script() {
    if (m_inited)
        callMethodNoArg("OnQuit");

    if (m_L && m_table_ref != LUA_NOREF) {
        lua_unref(m_L, m_table_ref);
        m_table_ref = LUA_NOREF;
    }
}
