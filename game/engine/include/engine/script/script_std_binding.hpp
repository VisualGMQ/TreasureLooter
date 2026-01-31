#pragma once
#include "angelscript.h"
#include "autowrapper/aswrappedcall.h"
#include "engine/script/script_macros.hpp"
#include <optional>
#include <string>

// Template function to register std::optional type
template <typename T>
void registerOptionalType(asIScriptEngine* engine,
                          const char* optional_type_name) {
    AS_CALL(engine->RegisterObjectType("Optional<class T>", 0,
                                       asOBJ_VALUE | asOBJ_TEMPLATE));
    engine->RegisterObjectBehaviour(
        "Optional<T>", asBEHAVE_FACTORY, "Optional<T>@ f(const T&in value)",
        asFUNCTIONPR([](asITypeInfo* type_info, void*) {
        }, (asITypeInfo*, void*), void),
        asCALL_CDECL_OBJLAST);
}

// Template function to bind std::optional type methods
template <typename T>
void bindOptionalType(asIScriptEngine* engine, const char* optional_type_name,
                      const char* value_type_name) {}
