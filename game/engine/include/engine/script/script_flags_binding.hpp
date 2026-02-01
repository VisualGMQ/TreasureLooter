#pragma once
#include "angelscript.h"
#include "engine/flag.hpp"
#include "engine/script/script_macros.hpp"

class CppFlags {
public:
    explicit CppFlags(asITypeInfo* type_info);
    CppFlags(asITypeInfo* type_info, int value);
    ~CppFlags();
    void SetValue(int value);
    int Value() const;
    void OpOrAssign(int value);
    void OpAndAssign(int value);
    void Remove(int value);

private:
    static bool IsIntegerSubType(int subtype_id);

    asITypeInfo* m_type_info = nullptr;
    int m_value = 0;
};

// Helpers for Flags<T> construct/destruct (implemented in .cpp)
void flagsDefaultConstruct(asITypeInfo* ti, void* obj);
void flagsConstructWithInt(asITypeInfo* ti, int value, void* obj);
void flagsDestruct(void* obj);

void registerFlagsType(asIScriptEngine* engine);
void bindFlagsType(asIScriptEngine* engine);

// Convert between Flags<EnumType> and CppFlags (for schema-generated bindings).
template <typename EnumType>
inline CppFlags CppFlagsFromFlags(asIScriptEngine* engine,
                                 const Flags<EnumType>& flags) {
    asITypeInfo* ti = engine->GetTypeInfoByDecl("TL::Flags");
    return CppFlags(ti, static_cast<int>(flags.Value()));
}

template <typename EnumType>
inline void FlagsFromCppFlags(const CppFlags* cpp_flags, Flags<EnumType>& out) {
    out = Flags<EnumType>(static_cast<typename Flags<EnumType>::underlying_type>(
        cpp_flags->Value()));
}
