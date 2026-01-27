#pragma once
#include "angelscript.h"
#include "autowrapper/aswrappedcall.h"
#include "engine/script_macros.hpp"
#include <optional>
#include <string>

// Template function to register std::optional type
template <typename T>
void registerOptionalType(asIScriptEngine* engine, const char* optional_type_name) {
    constexpr bool is_pod = std::is_standard_layout_v<std::optional<T>> && std::is_trivial_v<std::optional<T>>;
    AS_CALL(engine->RegisterObjectType(optional_type_name, sizeof(std::optional<T>),
                                       (is_pod ? asOBJ_POD : 0) | asOBJ_VALUE | asGetTypeTraits<std::optional<T>>()));
}

// Template function to bind std::optional type methods
template <typename T>
void bindOptionalType(asIScriptEngine* engine, const char* optional_type_name, const char* value_type_name) {
    std::string value_type_name_str(value_type_name);
    
    // Constructor - default (empty optional)
    AS_CALL(engine->RegisterObjectBehaviour(optional_type_name, asBEHAVE_CONSTRUCT,
                                            "void f()", WRAP_CON(std::optional<T>, ()),
                                            asCALL_GENERIC));
    
    // Constructor - from value
    std::string constructor_sig = "void f(const " + value_type_name_str + "& in)";
    AS_CALL(engine->RegisterObjectBehaviour(optional_type_name, asBEHAVE_CONSTRUCT,
                                            constructor_sig.c_str(),
                                            WRAP_CON(std::optional<T>, (const T&)),
                                            asCALL_GENERIC));
    
    // has_value() method
    AS_CALL(engine->RegisterObjectMethod(optional_type_name, "bool has_value() const",
                                         asMETHOD(std::optional<T>, has_value),
                                         asCALL_THISCALL));
    
    // value() method - returns the value
    std::string value_sig = "const " + value_type_name_str + "@ Value() const";
    AS_CALL(engine->RegisterObjectMethod(optional_type_name, value_sig.c_str(),
                                         asMETHODPR(std::optional<T>, value, () const, const T&),
                                         asCALL_THISCALL));
    
    // value_or() method - returns value or default
    std::string value_or_sig = value_type_name_str + " ValueOr(const " + value_type_name_str + "& in) const";
    AS_CALL(engine->RegisterObjectMethod(optional_type_name, value_or_sig.c_str(),
                                         asMETHODPR(std::optional<T>, value_or, (const T&) const, T),
                                         asCALL_THISCALL));
    
    // reset() method - makes optional empty
    AS_CALL(engine->RegisterObjectMethod(optional_type_name, "void Reset()",
                                         asMETHOD(std::optional<T>, reset),
                                         asCALL_THISCALL));
    
    // operator bool() - check if has value
    AS_CALL(engine->RegisterObjectMethod(optional_type_name, "bool opCast() const",
                                         asFUNCTION(+[](const std::optional<T>& opt) -> bool {
                                             return opt.has_value();
                                         }),
                                         asCALL_CDECL_OBJFIRST));
    
    // operator-> - access member (implicit cast to pointer)
    std::string arrow_sig = "const " + value_type_name_str + "@ opImplCast() const";
    AS_CALL(engine->RegisterObjectMethod(optional_type_name, arrow_sig.c_str(),
                                         asFUNCTION(+[](const std::optional<T>& opt) -> const T* {
                                             return opt.has_value() ? &opt.value() : nullptr;
                                         }),
                                         asCALL_CDECL_OBJFIRST));
}
