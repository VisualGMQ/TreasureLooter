#include "engine/script/script_option_binding.hpp"
#include "engine/macros.hpp"

#define AS_INNER_CALL(expr, msg)                     \
    do {                                             \
        if ((expr) < 0) {                            \
            asGetActiveContext()->SetException(msg); \
        }                                            \
    } while (0)

CppOptional::CppOptional(asITypeInfo* type_info) : m_type_info{type_info} {
    AS_INNER_CALL(type_info->AddRef(), "type_info add ref failed");
    assert(type_info->GetName() == std::string_view{"Optional"});
    int subtype_id = type_info->GetSubTypeId();

    if (subtype_id & asTYPEID_MASK_OBJECT) {
        m_elem_size = sizeof(asPWORD);
    } else {
        m_elem_size =
            m_type_info->GetEngine()->GetSizeOfPrimitiveType(subtype_id);
    }

    if (m_type_info->GetFlags() & asOBJ_GC) {
        m_type_info->GetEngine()->NotifyGarbageCollectorOfNewObject(
            this, m_type_info);
    }
}

CppOptional::~CppOptional() {
    Reset();
    if (m_type_info) m_type_info->Release();
}

void CppOptional::SetValue(void* value) {
    TL_RETURN_IF_NULL(value);
    auto subtype_id = m_type_info->GetSubTypeId();
    asIScriptEngine* engine = m_type_info->GetEngine();
    void* storage = reinterpret_cast<void*>(&m_value);

    if ((subtype_id & ~asTYPEID_MASK_SEQNBR) &&
        !(subtype_id & asTYPEID_OBJHANDLE)) {
        asITypeInfo* subtype = m_type_info->GetSubType();
        if (subtype->GetFlags() & asOBJ_ASHANDLE) {
            std::string decl = std::string(subtype->GetName()) +
                               "& opHndlAssign(const " +
                               std::string(subtype->GetName()) + "&in)";
            asIScriptFunction* func = subtype->GetMethodByDecl(decl.c_str());
            if (func) {
                asIScriptContext* ctx = engine->RequestContext();
                ctx->Prepare(func);
                ctx->SetObject(storage);
                ctx->SetArgAddress(0, value);
                ctx->Execute();
                engine->ReturnContext(ctx);
            } else {
                engine->AssignScriptObject(storage, value, subtype);
            }
        } else {
            if (!m_has_value || !m_value) {
                if (m_owns_buffer && m_value) {
                    destructValueType(engine, m_value, subtype);
                    asFreeMem(m_value);
                }
                m_value = asAllocMem(subtype->GetSize());
                m_owns_buffer = true;
            }
            engine->AssignScriptObject(m_value, value, subtype);
        }
        m_has_value = true;
    } else if (subtype_id & asTYPEID_OBJHANDLE) {
        void* old_ptr = m_has_value ? m_value : nullptr;
        m_value = *(void**)value;
        engine->AddRefScriptObject(m_value, m_type_info->GetSubType());
        if (old_ptr)
            engine->ReleaseScriptObject(old_ptr, m_type_info->GetSubType());
        m_has_value = true;
    } else {
        if (subtype_id == asTYPEID_BOOL || subtype_id == asTYPEID_INT8 ||
            subtype_id == asTYPEID_UINT8) {
            *(char*)storage = *(char*)value;
        } else if (subtype_id == asTYPEID_INT16 ||
                   subtype_id == asTYPEID_UINT16) {
            *(short*)storage = *(short*)value;
        } else if (subtype_id == asTYPEID_INT32 ||
                   subtype_id == asTYPEID_UINT32 ||
                   subtype_id == asTYPEID_FLOAT ||
                   subtype_id > asTYPEID_DOUBLE) {
            *(int*)storage = *(int*)value;
        } else if (subtype_id == asTYPEID_INT64 ||
                   subtype_id == asTYPEID_UINT64 ||
                   subtype_id == asTYPEID_DOUBLE) {
            *(double*)storage = *(double*)value;
        }
        m_has_value = true;
    }
}

bool CppOptional::Has() const {
    return m_has_value;
}

void* CppOptional::Value() const {
    return m_has_value ? const_cast<void*>(static_cast<const void*>(&m_value))
                       : nullptr;
}

void CppOptional::Reset() {
    if (!m_has_value) return;
    int subtype_id = m_type_info->GetSubTypeId();
    asIScriptEngine* engine = m_type_info->GetEngine();
    if (subtype_id & asTYPEID_OBJHANDLE) {
        if (m_value)
            engine->ReleaseScriptObject(m_value, m_type_info->GetSubType());
    } else if ((subtype_id & ~asTYPEID_MASK_SEQNBR) &&
               !(subtype_id & asTYPEID_OBJHANDLE)) {
        asITypeInfo* subtype = m_type_info->GetSubType();
        if (!(subtype->GetFlags() & asOBJ_ASHANDLE) && m_owns_buffer &&
            m_value) {
            destructValueType(engine, m_value, subtype);
            asFreeMem(m_value);
        }
    }
    m_value = nullptr;
    m_has_value = false;
    m_owns_buffer = false;
}

void CppOptional::destructValueType(asIScriptEngine* engine, void* ptr,
                                    asITypeInfo* subtype) {
    for (asUINT i = 0; i < subtype->GetBehaviourCount(); ++i) {
        asEBehaviours beh = asBEHAVE_CONSTRUCT;
        asIScriptFunction* func = subtype->GetBehaviourByIndex(i, &beh);
        if (beh != asBEHAVE_DESTRUCT || !func) continue;
        asIScriptContext* ctx = engine->RequestContext();
        ctx->Prepare(func);
        ctx->SetObject(ptr);
        ctx->Execute();
        engine->ReturnContext(ctx);
        break;
    }
}

void optionalDefaultConstruct(asITypeInfo* ti, void* obj) {
    new (obj) CppOptional(ti);
}

void optionalConstructWithValue(asITypeInfo* ti, void* value, void* obj) {
    new (obj) CppOptional(ti);
    static_cast<CppOptional*>(obj)->SetValue(value);
}

void optionalDestruct(void* obj) {
    static_cast<CppOptional*>(obj)->~CppOptional();
}

void registerOptionalType(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectType("Optional<class T>", sizeof(CppOptional),
                                       asOBJ_VALUE | asOBJ_TEMPLATE |
                                           asGetTypeTraits<CppOptional>()));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Optional<T>", asBEHAVE_CONSTRUCT, "void f(int &in)",
        asFUNCTIONPR(optionalDefaultConstruct, (asITypeInfo*, void*), void),
        asCALL_CDECL_OBJLAST));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Optional<T>", asBEHAVE_CONSTRUCT, "void f(int &in, const T&in value)",
        asFUNCTIONPR(optionalConstructWithValue, (asITypeInfo*, void*, void*),
                     void),
        asCALL_CDECL_OBJLAST));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Optional<T>", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTIONPR(optionalDestruct, (void*), void), asCALL_CDECL_OBJLAST));
}

void bindOptionalType(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod("Optional<T>", "bool Has() const",
                                         asMETHOD(CppOptional, Has),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Optional<T>", "T& Value() const",
        asMETHODPR(CppOptional, Value, () const, void*), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod("Optional<T>", "void Reset()",
                                         asMETHOD(CppOptional, Reset),
                                         asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Optional<T>", "void Set(const T&in value)",
        asMETHOD(CppOptional, SetValue), asCALL_THISCALL));
}