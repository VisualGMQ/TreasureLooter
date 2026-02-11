#include "engine/script/script_flags_binding.hpp"
#include <cassert>
#include <string_view>

#define AS_INNER_CALL(expr, msg)                     \
    do {                                             \
        if ((expr) < 0) {                            \
            asGetActiveContext()->SetException(msg); \
        }                                            \
    } while (0)

bool CppFlags::IsIntegerSubType(int subtype_id) {
    switch (subtype_id & asTYPEID_MASK_SEQNBR) {
    case asTYPEID_INT8:
    case asTYPEID_INT16:
    case asTYPEID_INT32:
    case asTYPEID_INT64:
    case asTYPEID_UINT8:
    case asTYPEID_UINT16:
    case asTYPEID_UINT32:
    case asTYPEID_UINT64:
        return true;
    default:
        break;
    }
    if (subtype_id > asTYPEID_DOUBLE)
        return true;
    return false;
}

CppFlags::CppFlags(asITypeInfo* type_info) : m_type_info{type_info} {
    AS_INNER_CALL(type_info->AddRef(), "type_info add ref failed");
    assert(type_info->GetName() == std::string_view{"Flags"});
    int subtype_id = type_info->GetSubTypeId();
    if (!IsIntegerSubType(subtype_id)) {
        asGetActiveContext()->SetException(
            "Flags<T> only accepts integer or enum subtype");
        return;
    }
}

CppFlags::CppFlags(asITypeInfo* type_info, int value) : m_type_info{type_info} {
    AS_INNER_CALL(type_info->AddRef(), "type_info add ref failed");
    assert(type_info->GetName() == std::string_view{"Flags"});
    int subtype_id = type_info->GetSubTypeId();
    if (!IsIntegerSubType(subtype_id)) {
        asGetActiveContext()->SetException(
            "Flags<T> only accepts integer or enum subtype");
        return;
    }
    m_value = value;
}

CppFlags::~CppFlags() {
    if (m_type_info)
        m_type_info->Release();
}

void CppFlags::SetValue(int value) {
    m_value = value;
}

int CppFlags::Value() const {
    return m_value;
}

void CppFlags::OpOrAssign(int value) {
    m_value |= value;
}

void CppFlags::OpAndAssign(int value) {
    m_value &= value;
}

void CppFlags::Remove(int value) {
    m_value &= ~value;
}

void flagsDefaultConstruct(asITypeInfo* ti, void* obj) {
    new (obj) CppFlags(ti);
}

void flagsConstructWithInt(asITypeInfo* ti, int value, void* obj) {
    new (obj) CppFlags(ti, value);
}

void flagsDestruct(void* obj) {
    static_cast<CppFlags*>(obj)->~CppFlags();
}

void registerFlagsType(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectType("Flags<class T>", sizeof(CppFlags),
                                       asOBJ_VALUE | asOBJ_TEMPLATE |
                                           asGetTypeTraits<CppFlags>()));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Flags<T>", asBEHAVE_CONSTRUCT, "void f(int &in)",
        asFUNCTIONPR(flagsDefaultConstruct, (asITypeInfo*, void*), void),
        asCALL_CDECL_OBJLAST));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Flags<T>", asBEHAVE_CONSTRUCT, "void f(int &in, int)",
        asFUNCTIONPR(flagsConstructWithInt, (asITypeInfo*, int, void*), void),
        asCALL_CDECL_OBJLAST));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Flags<T>", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTIONPR(flagsDestruct, (void*), void),
        asCALL_CDECL_OBJLAST));
}

void bindFlagsType(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod("Flags<T>", "int Value() const",
                                         asMETHOD(CppFlags, Value),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Flags<T>", "void opOrAssign(int)",
                                         asMETHOD(CppFlags, OpOrAssign),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Flags<T>", "void opAndAssign(int)",
                                         asMETHOD(CppFlags, OpAndAssign),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Flags<T>", "void Remove(int)",
                                         asMETHOD(CppFlags, Remove),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Flags<T>", "void Set(int)",
                                         asMETHOD(CppFlags, SetValue),
                                         asCALL_THISCALL));
}
