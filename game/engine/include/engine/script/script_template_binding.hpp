#pragma once
#include "angelscript.h"
#include "autowrapper/aswrappedcall.h"
#include "engine/flag.hpp"
#include "engine/handle.hpp"
#include "engine/math.hpp"
#include "engine/script/script_macros.hpp"
#include <string>

// Template function to register TVec2 type
template <typename T>
void registerTVec2Type(asIScriptEngine* engine, const char* type_name) {
    constexpr bool is_float = std::is_same_v<T, float>;
    AS_CALL(engine->RegisterObjectType(
        type_name, sizeof(TVec2<T>),
        asOBJ_POD | asOBJ_VALUE | (is_float ? asOBJ_APP_CLASS_ALLFLOATS : 0) |
            asGetTypeTraits<TVec2<T>>()));
}

// Template function to bind TVec2 type methods
template <typename T>
void bindTVec2Type(asIScriptEngine* engine, const std::string& type_name,
                   const std::string& element_type_name) {
    // Constructors
    AS_CALL(engine->RegisterObjectBehaviour(
        type_name.c_str(), asBEHAVE_CONSTRUCT, "void f()",
        WRAP_CON(TVec2<T>, ()), asCALL_GENERIC));
    std::string constructorSig =
        "void f(" + element_type_name + ", " + element_type_name + ")";
    AS_CALL(engine->RegisterObjectBehaviour(
        type_name.c_str(), asBEHAVE_CONSTRUCT, constructorSig.c_str(),
        WRAP_CON(TVec2<T>, (T, T)), asCALL_GENERIC));

    // Properties
    AS_CALL(engine->RegisterObjectProperty(type_name.c_str(),
                                           (element_type_name + " x").c_str(),
                                           asOFFSET(TVec2<T>, x)));
    AS_CALL(engine->RegisterObjectProperty(type_name.c_str(),
                                           (element_type_name + " y").c_str(),
                                           asOFFSET(TVec2<T>, y)));
    AS_CALL(engine->RegisterObjectProperty(type_name.c_str(),
                                           (element_type_name + " w").c_str(),
                                           asOFFSET(TVec2<T>, w)));
    AS_CALL(engine->RegisterObjectProperty(type_name.c_str(),
                                           (element_type_name + " h").c_str(),
                                           asOFFSET(TVec2<T>, h)));

    // Operators
    std::string opEqualsSig = "bool opEquals(const " + type_name + "&in) const";
    AS_CALL(engine->RegisterObjectMethod(
        type_name.c_str(), opEqualsSig.c_str(),
        asMETHODPR(TVec2<T>, operator==, (const TVec2<T>&) const, bool),
        asCALL_THISCALL));
    std::string opNotEqualsSig =
        "bool opNotEquals(const " + type_name + "&in) const";
    AS_CALL(engine->RegisterObjectMethod(
        type_name.c_str(), opNotEqualsSig.c_str(),
        asMETHODPR(TVec2<T>, operator!=, (const TVec2<T>&) const, bool),
        asCALL_THISCALL));

    // Additional operators and methods for all types
    std::string opAddAssignSig =
        type_name + "& opAddAssign(const " + type_name + "& in)";
    AS_CALL(engine->RegisterObjectMethod(
        type_name.c_str(), opAddAssignSig.c_str(),
        asMETHODPR(TVec2<T>, operator+=, (const TVec2<T>&), TVec2<T>&),
        asCALL_THISCALL));
    std::string opSubAssignSig =
        type_name + "& opSubAssign(const " + type_name + "& in)";
    AS_CALL(engine->RegisterObjectMethod(
        type_name.c_str(), opSubAssignSig.c_str(),
        asMETHODPR(TVec2<T>, operator-=, (const TVec2<T>&), TVec2<T>&),
        asCALL_THISCALL));
    std::string opAddSig = type_name + " opAdd(const " + type_name + "& in)";
    AS_CALL(engine->RegisterObjectMethod(
        type_name.c_str(), opAddSig.c_str(),
        asMETHODPR_CONST(TVec2<T>, operator+, (const TVec2<T>&), TVec2<T>),
        asCALL_THISCALL));
    std::string opSubSig = type_name + " opSub(const " + type_name + "& in)";
    AS_CALL(engine->RegisterObjectMethod(
        type_name.c_str(), opSubSig.c_str(),
        asMETHODPR_CONST(TVec2<T>, operator-, (const TVec2<T>&), TVec2<T>),
        asCALL_THISCALL));

    // Methods available for all TVec2 types
    std::string dotSig = "float Dot(const " + type_name + "& in) const";
    AS_CALL(engine->RegisterObjectMethod(type_name.c_str(), dotSig.c_str(),
                                         asMETHOD(TVec2<T>, Dot),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        type_name.c_str(), "float Length() const", asMETHOD(TVec2<T>, Length),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        type_name.c_str(), "float LengthSquared() const",
        asMETHOD(TVec2<T>, LengthSquared), asCALL_THISCALL));
    std::string normalizeSig = type_name + " Normalize() const";
    AS_CALL(engine->RegisterObjectMethod(
        type_name.c_str(), normalizeSig.c_str(), asMETHOD(TVec2<T>, Normalize),
        asCALL_THISCALL));

    // Additional operators for float types
    if constexpr (std::is_same_v<T, float>) {
        AS_CALL(engine->RegisterObjectMethod(
            type_name.c_str(), "Vec2& opMulAssign(const Vec2& in)",
            asMETHODPR(TVec2<T>, operator*=, (const TVec2<T>&), TVec2<T>&),
            asCALL_THISCALL));
        AS_CALL(engine->RegisterObjectMethod(
            type_name.c_str(), "Vec2& opDivAssign(const Vec2& in)",
            asMETHODPR(TVec2<T>, operator/=, (const TVec2<T>&), TVec2<T>&),
            asCALL_THISCALL));
        AS_CALL(engine->RegisterObjectMethod(
            type_name.c_str(), "Vec2 opMul(const Vec2& in) const",
            asMETHODPR_CONST(TVec2<T>, operator*, (const TVec2<T>&), TVec2<T>),
            asCALL_THISCALL));
        AS_CALL(engine->RegisterObjectMethod(
            type_name.c_str(), "Vec2 opMul(float) const",
            asMETHODPR_CONST(TVec2<T>, operator*, (float), TVec2<T>),
            asCALL_THISCALL));
        AS_CALL(engine->RegisterObjectMethod(
            type_name.c_str(), "Vec2 opMul(double) const",
            asFUNCTION(+[](const TVec2<T>* v, double s) -> TVec2<T> {
                return (*v) * static_cast<float>(s);
            }),
            asCALL_CDECL_OBJFIRST));
        AS_CALL(engine->RegisterObjectMethod(
            type_name.c_str(), "Vec2 opDiv(const Vec2& in) const",
            asMETHODPR_CONST(TVec2<T>, operator/, (const TVec2<T>&), TVec2<T>),
            asCALL_THISCALL));
        AS_CALL(engine->RegisterObjectMethod(
            type_name.c_str(), "Vec2 opDiv(float) const",
            asMETHODPR_CONST(TVec2<T>, operator/, (float), TVec2<T>),
            asCALL_THISCALL));
        AS_CALL(engine->RegisterObjectMethod(
            type_name.c_str(), "Vec2 opMul_r(float) const",
            asFUNCTIONPR(operator*, (float, const TVec2<T>&), TVec2<T>),
            asCALL_CDECL_OBJLAST));
        AS_CALL(engine->RegisterObjectMethod(
            type_name.c_str(), "Vec2 opMul_r(double) const",
            asFUNCTION(+[](double s, const TVec2<T>* v) -> TVec2<T> {
                return static_cast<float>(s) * (*v);
            }),
            asCALL_CDECL_OBJLAST));
    }
}

// Template function to register Flags type
template <typename EnumType>
void registerFlagsType(asIScriptEngine* engine, const char* type_name) {
    AS_CALL(engine->RegisterObjectType(
        type_name, sizeof(Flags<EnumType>),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Flags<EnumType>>()));
}

// Template function to bind Flags type methods
template <typename EnumType>
void bindFlagsType(asIScriptEngine* engine, const char* type_name) {
    // Constructors
    AS_CALL(engine->RegisterObjectBehaviour(
        type_name, asBEHAVE_CONSTRUCT, "void f()",
        WRAP_CON(Flags<EnumType>, ()), asCALL_GENERIC));
    AS_CALL(engine->RegisterObjectBehaviour(
        type_name, asBEHAVE_CONSTRUCT, "void f(int)",
        asFUNCTION(+[](void* mem, int value) {
            new (mem) Flags<EnumType>(
                static_cast<typename Flags<EnumType>::underlying_type>(value));
        }),
        asCALL_CDECL_OBJFIRST));

    // Value() method
    AS_CALL(engine->RegisterObjectMethod(type_name, "int Value() const",
                                         asMETHOD(Flags<EnumType>, Value),
                                         asCALL_THISCALL));

    // Operator |=
    AS_CALL(engine->RegisterObjectMethod(
        type_name, "void opOrAssign(int)",
        asFUNCTION(+[](Flags<EnumType>* f,
                       typename Flags<EnumType>::underlying_type v) {
            *f |= static_cast<EnumType>(v);
        }),
        asCALL_CDECL_OBJFIRST));

    // Operator &=
    AS_CALL(engine->RegisterObjectMethod(
        type_name, "void opAndAssign(int)",
        asFUNCTION(+[](Flags<EnumType>* f,
                       typename Flags<EnumType>::underlying_type v) {
            *f &= static_cast<EnumType>(v);
        }),
        asCALL_CDECL_OBJFIRST));

    // Remove method
    AS_CALL(engine->RegisterObjectMethod(
        type_name, "void Remove(int)",
        asFUNCTION(+[](Flags<EnumType>* f,
                       typename Flags<EnumType>::underlying_type v) {
            f->Remove(static_cast<EnumType>(v));
        }),
        asCALL_CDECL_OBJFIRST));
}

// Template function to register Handle type
template <typename T>
void registerHandleType(asIScriptEngine* engine, const char* handle_type_name) {
    AS_CALL(engine->RegisterObjectType(
        handle_type_name, sizeof(Handle<T>),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Handle<T>>()));
}

// Template function to bind Handle type methods
template <typename T>
void bindHandleType(asIScriptEngine* engine, const char* handle_type_name,
                    const char* underlying_type_name) {
    // Constructor
    AS_CALL(engine->RegisterObjectBehaviour(
        handle_type_name, asBEHAVE_CONSTRUCT, "void f()",
        WRAP_CON(Handle<T>, ()), asCALL_GENERIC));

    // Operators
    AS_CALL(engine->RegisterObjectMethod(
        handle_type_name,
        (std::string("bool opEquals(const ") + handle_type_name + "&in)")
            .c_str(),
        asMETHODPR(Handle<T>, operator==, (const Handle<T>&), bool),
        asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        handle_type_name,
        (std::string("bool opNotEquals(const ") + handle_type_name + "&in)")
            .c_str(),
        asMETHODPR(Handle<T>, operator!=, (const Handle<T>&), bool),
        asCALL_THISCALL));

    // Implicit cast to underlying type pointer
    std::string cast_sig = std::string(underlying_type_name) + "@ opImplCast()";
    AS_CALL(engine->RegisterObjectMethod(
        handle_type_name, cast_sig.c_str(),
        asFUNCTION(+[](Handle<T>& h) -> T* { return h.Get(); }),
        asCALL_CDECL_OBJLAST));
}
