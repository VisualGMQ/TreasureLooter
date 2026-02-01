#include "engine/script/script.hpp"
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
#include "scripthandle/scripthandle.h"
#include "scriptmath/scriptmath.h"
#include "scriptmath/scriptmathcomplex.h"
#include "scriptstdstring/scriptstdstring.h"
#include "weakref/weakref.h"
#include "engine/script/script_macros.hpp"
#include "engine/script/script_binding.hpp"
#include <cassert>
#include <cstring>

ScriptBinaryData::ScriptBinaryData(const Path& filename,
                                   asIScriptEngine* engine) {
    auto io = IOStream::CreateFromFile(filename, IOMode::Read, true);
    m_content = io->Read();

    CScriptBuilder builder;
    AS_CALL_WITH_RETURN(builder.StartNewModule(engine, "module"));

    std::string filename_str = filename.string();
    static constexpr const char* kTLBehaviorExternal =
        "namespace TL { external shared class Behavior; }";
    AS_CALL_WITH_RETURN_AND_MSG(
        builder.AddSectionFromMemory("tl_behavior_external", kTLBehaviorExternal,
                                     std::strlen(kTLBehaviorExternal)),
        "load tl behavior external script failed");
    AS_CALL_WITH_RETURN_AND_MSG(
        builder.AddSectionFromMemory(filename_str.c_str(), m_content.data(),
                                     m_content.size()),
        "load script failed");

    AS_CALL_WITH_RETURN_AND_MSG(builder.BuildModule(), "build module failed");
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
    RegisterScriptHandle(m_engine);
    RegisterScriptWeakRef(m_engine);
    RegisterScriptMath(m_engine);
    RegisterScriptMathComplex(m_engine);

    bindModule();

    buildSharedModule();
}

void ScriptBinaryDataManager::bindModule() {
    BindTLModule(m_engine);
}

ScriptBinaryDataManager::~ScriptBinaryDataManager() {
    AssetManagerBase<ScriptBinaryData>::Clear();
    m_engine->ShutDownAndRelease();
}

ScriptBinaryDataHandle ScriptBinaryDataManager::Load(const Path& filename,
                                                     bool force) {
    if (auto it = Find(filename); it && !force) {
        LOGW("script binary data {} already loaded", filename);
        return it;
    }

    return store(&filename, UUID::CreateV4(),
                 std::make_unique<ScriptBinaryData>(filename, m_engine));
}

void ScriptBinaryDataManager::buildSharedModule() {
    auto behavior_io =
        IOStream::CreateFromFile(Path("script/TLBehavior.as"), IOMode::Read, true);
    TL_RETURN_IF_NULL_WITH_LOG(behavior_io, LOGE,
                               "script/TLBehavior.as not found");
    std::vector<char> behavior_content = behavior_io->Read();

    CScriptBuilder builder;
    AS_CALL_WITH_RETURN(builder.StartNewModule(m_engine, "shared"));
    AS_CALL_WITH_RETURN_AND_MSG(
        builder.AddSectionFromMemory("tl_behavior", behavior_content.data(),
                                     behavior_content.size()),
        "load tl behavior shared script failed");
    AS_CALL_WITH_RETURN_AND_MSG(builder.BuildModule(),
                                "build shared module failed");
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

Script::Script(Entity entity, ScriptBinaryDataHandle handle) {
    TL_RETURN_IF_FALSE(handle);

    auto engine =
        CURRENT_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>()
            .GetUnderlyingEngine();

    asIScriptModule* mod = engine->GetModule("module");
    asITypeInfo* type = mod->GetTypeInfoByDecl("MyClass");

    TL_RETURN_IF_NULL_WITH_LOG(
        type, LOGE, "[AngelScript]: script {} don't has MyClass",
        handle.GetFilename() ? handle.GetFilename()->string() : "<anonymouse>");

    m_init_fn = type->GetMethodByDecl("void OnInit()");
    m_update_fn = type->GetMethodByDecl("void OnUpdate(TL::TimeType)");
    m_quit_fn = type->GetMethodByDecl("void OnQuit()");

    m_ctx = engine->CreateContext();
    TL_RETURN_IF_NULL_WITH_LOG(m_ctx, LOGE,
                               "[AngelScript]: script context create failed");

    asIScriptFunction* factory =
        type->GetFactoryByDecl("MyClass@ f(TL::Entity)");
    TL_RETURN_IF_NULL_WITH_LOG(factory, LOGE,
                               "[AngelScript]: class {} missing ctor(Entity)",
                               "MyClass");

    m_ctx->Prepare(factory);
    m_ctx->SetArgObject(0, &entity);
    int ctor_r = m_ctx->Execute();
    if (ctor_r != asEXECUTION_FINISHED) {
        if (ctor_r == asEXECUTION_EXCEPTION) {
            LOGE("[AngelScript Execute] An exception '{}' occurred",
                 m_ctx->GetExceptionString());
        }
        return;
    }

    m_class_instance =
        *static_cast<asIScriptObject**>(m_ctx->GetAddressOfReturnValue());
    if (m_class_instance) {
        AS_CALL(m_class_instance->AddRef());
    }

    TL_RETURN_IF_NULL_WITH_LOG(m_class_instance, LOGE,
                               "[AngelScript]: class {} can't instantiate",
                               "MyClass");
}

void Script::Update() {
    TL_RETURN_IF_FALSE(m_ctx);

    if (!m_inited) {
        callNoArgMethod(m_init_fn);
        m_inited = true;
    }

    callUpdateMethod(m_update_fn, CURRENT_CONTEXT.m_time->GetElapseTime());
}

void Script::callNoArgMethod(asIScriptFunction* fn) {
    TL_RETURN_IF_NULL(fn);
    TL_RETURN_IF_NULL(m_class_instance);

    m_ctx->Prepare(fn);
    m_ctx->SetObject(m_class_instance);
    int r = m_ctx->Execute();
    if (r != asEXECUTION_FINISHED) {
        if (r == asEXECUTION_EXCEPTION) {
            LOGE("[AngelScript Execute] An exception '{}' occurred",
                 m_ctx->GetExceptionString());
        }
    }
}

void Script::callUpdateMethod(asIScriptFunction* fn, TimeType delta_time) {
    TL_RETURN_IF_NULL(fn);
    TL_RETURN_IF_NULL(m_class_instance);

    m_ctx->Prepare(fn);
    m_ctx->SetObject(m_class_instance);
    m_ctx->SetArgDouble(0, delta_time);
    int r = m_ctx->Execute();
    if (r != asEXECUTION_FINISHED) {
        if (r == asEXECUTION_EXCEPTION) {
            LOGE("[AngelScript Execute] An exception '{}' occurred",
                 m_ctx->GetExceptionString());
        }
    }
}

Script::~Script() {
    if (m_inited) {
        callNoArgMethod(m_quit_fn);
    }

    if (m_class_instance) {
        AS_CALL(m_class_instance->Release());
    }
    if (m_ctx) {
        AS_CALL(m_ctx->Release());
    }
}
