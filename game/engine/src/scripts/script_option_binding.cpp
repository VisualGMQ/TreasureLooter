#include "engine/script/script_option_binding.hpp"
#include "engine/macros.hpp"
#include <cstring>

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

CppOptional::CppOptional(const CppOptional& other)
    : m_type_info{other.m_type_info},
      m_elem_size{other.m_elem_size},
      m_has_value{false},
      m_owns_buffer{false} {
    m_value = nullptr;
    if (m_type_info) m_type_info->AddRef();
    if (!other.m_has_value) return;

    int subtype_id = m_type_info->GetSubTypeId();
    asITypeInfo* subtype = m_type_info->GetSubType();

    if (subtype_id & asTYPEID_OBJHANDLE) {
        SetValue(const_cast<void*>(static_cast<const void*>(&other.m_value)));
    } else if ((subtype_id & ~asTYPEID_MASK_SEQNBR) &&
               !(subtype_id & asTYPEID_OBJHANDLE)) {
        if (subtype->GetFlags() & asOBJ_ASHANDLE) {
            SetValue(const_cast<void*>(static_cast<const void*>(&other.m_value)));
        } else {
            SetValue(other.m_value);
        }
    } else {
        // 基本类型：值在 other.m_value 指向的缓冲区
        SetValue(other.m_value);
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
        // 基本类型：也存到堆缓冲区，使 Value() 始终返回 m_value，脚本读到的地址稳定
        size_t prim_size =
            engine->GetSizeOfPrimitiveType(subtype_id);
        if (!m_has_value || !m_value) {
            if (m_owns_buffer && m_value) {
                asFreeMem(m_value);
            }
            m_value = asAllocMem(static_cast<size_t>(prim_size));
            m_owns_buffer = true;
        }
        std::memcpy(m_value, value, prim_size);
        m_has_value = true;
    }
}

bool CppOptional::Has() const {
    return m_has_value;
}

void* CppOptional::Value() const {
    if (!m_has_value) return nullptr;
    if (m_owns_buffer)
        return m_value;
    return const_cast<void*>(static_cast<const void*>(&m_value));
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
        if (subtype->GetFlags() & asOBJ_ASHANDLE) {
            // ASHANDLE 存 union，不释放
        } else if (m_owns_buffer && m_value) {
            destructValueType(engine, m_value, subtype);
            asFreeMem(m_value);
        }
    } else {
        // 基本类型：堆缓冲区由我们分配，需要释放
        if (m_owns_buffer && m_value)
            asFreeMem(m_value);
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

void optionalCopyConstruct(const CppOptional* other, void* obj) {
    new (obj) CppOptional(*other);
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
        "Optional<T>", asBEHAVE_CONSTRUCT,
        "void f(int &in, const Optional<T>&in other)",
        asFUNCTIONPR(optionalCopyConstruct, (const CppOptional*, void*), void),
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