#include "engine/script/script.hpp"
#include "angelscript.h"
#include "engine/asset_manager.hpp"
#include "engine/context.hpp"
#include "engine/log.hpp"
#include "engine/macros.hpp"
#include "engine/script/script_binding.hpp"
#include "engine/script/script_macros.hpp"
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
#include <cassert>

ScriptBinaryData::ScriptBinaryData(const Path& filename,
                                   asIScriptEngine* engine) {
    auto io = IOStream::CreateFromFile(filename, IOMode::Read, true);
    m_content = io->Read();

    TL_RETURN_IF_FALSE_WITH_LOG(!m_content.empty(), LOGE,
                                "read script {} failed", filename);

    CScriptBuilder builder;
    AS_CALL_WITH_RETURN(builder.StartNewModule(engine, "module"));

    std::string filename_str = filename.string();
    AS_CALL_WITH_RETURN_AND_MSG(
        builder.AddSectionFromMemory(filename_str.c_str(), m_content.data(),
                                     m_content.size()),
        "load script failed");

    AS_CALL_WITH_RETURN_AND_MSG(builder.BuildModule(), "build module failed");

    auto module = builder.GetModule();
    TL_RETURN_IF_FALSE(module);

    // firstly, using reflection to get class name
    // failed, due to #include may include multiple class inherit from
    // TL::Behavior so finally we use filename(no - & _, first character
    // uppercase) as class name
    TL_RETURN_IF_TRUE(filename_str.empty());

    m_class_name = filename.filename().replace_extension("").string();
    m_class_name[0] = std::toupper(m_class_name[0]);
    size_t i = 1;
    while (i < m_class_name.size()) {
        if ((m_class_name[i] == '-' || m_class_name[i] == '_')) {
            if (i + 1 < m_class_name.size()) {
                m_class_name[i + 1] = std::toupper(m_class_name[i + 1]);
            }
            m_class_name.erase(m_class_name.begin() + i);
        }
        i++;
    }
}

const std::vector<char>& ScriptBinaryData::GetContent() const {
    return m_content;
}

const std::string& ScriptBinaryData::GetClassName() const {
    return m_class_name;
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
    RegisterScriptArray(m_engine, true);
    RegisterScriptDictionary(m_engine);
    RegisterScriptHandle(m_engine);
    RegisterScriptWeakRef(m_engine);
    RegisterScriptMath(m_engine);
    RegisterScriptMathComplex(m_engine);

    bindModule();
}

void ScriptBinaryDataManager::bindModule() {
    BindTLModule(m_engine);
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
                 std::make_unique<ScriptBinaryData>(filename, m_engine));
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

    auto& class_name = handle->GetClassName();

    LOGI("class name: {}", class_name);
    asITypeInfo* type = mod->GetTypeInfoByDecl(class_name.c_str());

    TL_ASSERT(!class_name.empty());
    TL_ASSERT(type);

    m_init_fn = type->GetMethodByDecl("void OnInit()");
    m_update_fn = type->GetMethodByDecl("void OnUpdate(TL::TimeType)");
    m_quit_fn = type->GetMethodByDecl("void OnQuit()");

    m_ctx = engine->CreateContext();
    TL_RETURN_IF_NULL_WITH_LOG(m_ctx, LOGE,
                               "[AngelScript]: script context create failed");

    std::string factory_method_name = class_name + "@ f(TL::Entity)";
    asIScriptFunction* factory =
        type->GetFactoryByDecl(factory_method_name.c_str());
    TL_RETURN_IF_NULL_WITH_LOG(
        factory, LOGE, "[AngelScript]: class {} missing ctor(Entity) {}",
        class_name);

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
                               "[AngelScript]: class {} can't instantiate {}",
                               class_name);
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
