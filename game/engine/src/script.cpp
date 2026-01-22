#include "engine/script.hpp"
#include "engine/log.hpp"
#include "angelscript.h"
#include "scriptbuilder/scriptbuilder.h"
#include <cassert>

// Implement a simple message callback function
void MessageCallback(const asSMessageInfo* msg, void* param) {
    const char* type = "ERR ";
    if (msg->type == asMSGTYPE_WARNING)
        type = "WARN";
    else if (msg->type == asMSGTYPE_INFORMATION)
        type = "INFO";
    LOGI("{} ({}, {}) : {} : {}", msg->section, msg->row, msg->col, type,
           msg->message);
}

void ScriptManager::testRun() {
    asIScriptEngine* engine = asCreateScriptEngine();
    engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);

    RegisterStdString(engine);
    // RegisterStdStringUtils(engine);

    CScriptBuilder builder;
    int r = builder.StartNewModule(engine, "MyModule");
    if (r < 0) {
        LOGE("angelscript start module failed");
        return;
    }

    r = builder.AddSectionFromFile("script/test.as");
    if (r < 0) {
        LOGE("angelscript load script failed");
        return;
    }

    r = builder.BuildModule();
    if (r < 0) {
        LOGE("angelscript build module failed");
        return;
    }

    asIScriptModule* mod = engine->GetModule("MyModule", asGM_ALWAYS_CREATE);
    asIScriptFunction* func = mod->GetFunctionByDecl("void main()");

    if (func == 0) {
        LOGE("no void main() function in module");
    }

    asIScriptContext* ctx = engine->CreateContext();
    ctx->Prepare(func);
    r = ctx->Execute();
    if (r != asEXECUTION_FINISHED) {
        if( r == asEXECUTION_EXCEPTION )
        {
            // An exception occurred, let the script writer know what happened so it can be corrected.
            LOGE("An exception '{}' occurred. Please correct the code and try again.", ctx->GetExceptionString());
        }
    }

    ctx->Release();
    engine->ShutDownAndRelease();
}


