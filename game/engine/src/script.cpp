#include "engine/script.hpp"
#include "angelscript.h"
#include "engine/asset_manager.hpp"
#include "engine/context.hpp"
#include "engine/log.hpp"
#include "engine/macros.hpp"
#include "engine/storage.hpp"
#include "scriptany/scriptany.h"
#include "scriptarray/scriptarray.h"
#include "scriptbuilder/scriptbuilder.h"
#include "scriptdictionary/scriptdictionary.h"
#include "scriptstdstring/scriptstdstring.h"
#include <cassert>

ScriptBinaryData::ScriptBinaryData(const Path& filename) {
    auto io = IOStream::CreateFromFile(filename, IOMode::Read, false);
    m_content = io->Read();
}

const std::vector<char>& ScriptBinaryData::GetContent() const {
    return m_content;
}

ScriptBinaryData::~ScriptBinaryData() {}

void AngelScriptMessageCallback(const asSMessageInfo* msg, void* param) {
    const char* type = "ERR ";
    if (msg->type == asMSGTYPE_WARNING)
        type = "WARN";
    else if (msg->type == asMSGTYPE_INFORMATION)
        type = "INFO";
    LOGI("{} ({}, {}) : {} : {}", msg->section, msg->row, msg->col, type,
         msg->message);
}

ScriptBinaryDataManager::ScriptBinaryDataManager() {
    m_engine = asCreateScriptEngine();
    if (!m_engine) {
        LOGE("AngelScript engine init failed!");
        return;
    }
    m_engine->SetMessageCallback(asFUNCTION(AngelScriptMessageCallback), 0,
                                 asCALL_CDECL);

    RegisterStdString(m_engine);
    RegisterScriptAny(m_engine);
    RegisterScriptArray(m_engine, false);
    RegisterScriptDictionary(m_engine);

    bindModule();
}

// test function
void Print(const std::string& msg) {
    LOGI("[AngelScript]: {}", msg);
}

void ScriptBinaryDataManager::bindModule() {
    TL_RETURN_IF_NULL_WITH_LOG(m_engine, LOGE, "angel script engine is null!");
    int r = m_engine->RegisterGlobalFunction("void print(const string)", asFUNCTION(Print),
                                           asCALL_CDECL);
    TL_RETURN_IF_FALSE_WITH_LOG(r >= 0, LOGE, "register function failed");
}

ScriptBinaryDataManager::~ScriptBinaryDataManager() {
    m_engine->ShutDownAndRelease();
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

asIScriptEngine* ScriptBinaryDataManager::GetUnderlyingEngine() {
    return m_engine;
}

ScriptComponentManager::ScriptComponentManager() {}

ScriptComponentManager::~ScriptComponentManager() {}

void ScriptComponentManager::Update() {
    for (auto&& [entity, component] : m_components) {
        component.m_component->Update();
    }
}

Script::Script(ScriptBinaryDataHandle handle) {
    TL_RETURN_IF_FALSE(handle);

    auto engine =
        CURRENT_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>()
            .GetUnderlyingEngine();

    CScriptBuilder builder;
     int r = builder.StartNewModule(engine, "module");
     if (r < 0) {
         LOGE("angelscript start module failed");
         return;
     }

    auto filename = handle.GetFilename();
     std::string filename_str = filename ? filename->string() : "<anonymouse>";
    r = builder.AddSectionFromMemory(
        filename_str.c_str(),
        handle->GetContent().data(), handle->GetContent().size());
    TL_RETURN_IF_FALSE_WITH_LOG(r >= 0, LOGE, "angelscript load script failed");

    r = builder.BuildModule();
    if (r < 0) {
        LOGE("angelscript build module failed");
        return;
    }

    asIScriptModule* mod = engine->GetModule("module");
    asITypeInfo* type = mod->GetTypeInfoByDecl("MyClass");

    TL_RETURN_IF_NULL_WITH_LOG(type, LOGE, "script {} don't has MyClass",
                               filename_str);

    m_init_fn = type->GetMethodByDecl("void OnInit()");
    m_update_fn = type->GetMethodByDecl("void OnUpdate(float)");
    m_quit_fn = type->GetMethodByDecl("void OnQuit()");

    m_ctx = engine->CreateContext();
    if (!m_ctx) {
        LOGE("script context create failed");
    }

    m_class_instance = (asIScriptObject*)engine->CreateScriptObject(type);
    m_class_instance->AddRef();
}

void Script::Update() {
    TL_RETURN_IF_FALSE(m_update_fn && m_ctx);

    m_ctx->Prepare(m_update_fn);
    m_ctx->SetObject(m_class_instance);
    int r = m_ctx->Execute();
    if (r != asEXECUTION_FINISHED) {
        if (r == asEXECUTION_EXCEPTION) {
            LOGE("[AngelScript Execute] An exception '{}' occurred",
                 m_ctx->GetExceptionString());
        }
    }
}

Script::~Script() {
    m_class_instance->Release();
    m_ctx->Release();
}
