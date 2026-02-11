#include "engine/script/script_handle_binding.hpp"
#include "angelscript.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

static_assert(sizeof(Handle<uint8_t>) <= kCppHandleStorageSize,
              "Handle<T> storage too small");

#define AS_INNER_CALL(expr, msg)                     \
    do {                                             \
        if ((expr) < 0) {                            \
            asGetActiveContext()->SetException(msg); \
        }                                            \
    } while (0)

CppHandle::CppHandle(asITypeInfo* type_info) : m_type_info{type_info} {
    if (type_info) {
        AS_INNER_CALL(type_info->AddRef(), "type_info add ref failed");
        assert(type_info->GetName() == std::string_view{"Handle"});
    }
    std::memset(m_storage, 0, kCppHandleStorageSize);
}

CppHandle::CppHandle(asITypeInfo* type_info, void* handle_ptr)
    : m_type_info{type_info} {
    if (type_info) {
        AS_INNER_CALL(type_info->AddRef(), "type_info add ref failed");
        assert(type_info->GetName() == std::string_view{"Handle"});
    }
    std::memset(m_storage, 0, kCppHandleStorageSize);
    if (handle_ptr) {
        std::memcpy(m_storage, handle_ptr, sizeof(Handle<uint8_t>));
    }
}

CppHandle::CppHandle(const CppHandle& other) : m_type_info{other.m_type_info} {
    if (m_type_info)
        m_type_info->AddRef();
    std::memcpy(m_storage, other.m_storage, kCppHandleStorageSize);
}

CppHandle::~CppHandle() {
    if (m_type_info)
        m_type_info->Release();
}

void* CppHandle::Get() const {
    return *reinterpret_cast<void**>(const_cast<char*>(m_storage));
}

void* CppHandle::GetHandlePtr() const {
    return const_cast<char*>(m_storage);
}

CppHandle& CppHandle::opAssign(const CppHandle& other) {
    if (this != &other)
        std::memcpy(GetHandlePtr(), other.GetHandlePtr(),
                    sizeof(kCppHandleStorageSize));
    return *this;
}

bool CppHandle::opEquals(const CppHandle& other) const {
    return Get() == other.Get();
}

bool CppHandle::opNotEquals(const CppHandle& other) const {
    return !opEquals(other);
}

void handleDefaultConstruct(asITypeInfo* ti, void* obj) {
    new (obj) CppHandle(ti);
}

void handleConstructFromPtr(asITypeInfo* ti, void* handle_ptr, void* obj) {
    new (obj) CppHandle(ti);
    auto* dst = static_cast<CppHandle*>(obj);
    if (!handle_ptr)
        return;
    auto* src = static_cast<CppHandle*>(handle_ptr);
    std::memcpy(dst->GetHandlePtr(), src->GetHandlePtr(),
                sizeof(Handle<uint8_t>));
}

void handleDestruct(void* obj) {
    static_cast<CppHandle*>(obj)->~CppHandle();
}

void registerHandleType(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectType("Handle<class T>", sizeof(CppHandle),
                                       asOBJ_VALUE | asOBJ_TEMPLATE |
                                           asGetTypeTraits<CppHandle>()));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Handle<T>", asBEHAVE_CONSTRUCT, "void f(int &in)",
        asFUNCTIONPR(handleDefaultConstruct, (asITypeInfo*, void*), void),
        asCALL_CDECL_OBJLAST));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Handle<T>", asBEHAVE_CONSTRUCT, "void f(int &in, const Handle<T>&in)",
        asFUNCTIONPR(handleConstructFromPtr, (asITypeInfo*, void*, void*), void),
        asCALL_CDECL_OBJLAST));
    AS_CALL(engine->RegisterObjectBehaviour(
        "Handle<T>", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTIONPR(handleDestruct, (void*), void), asCALL_CDECL_OBJLAST));
}

void bindHandleType(asIScriptEngine* engine) {
    AS_CALL(engine->RegisterObjectMethod(
        "Handle<T>", "Handle<T>& opAssign(const Handle<T>& in)",
        asMETHOD(CppHandle, opAssign), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Handle<T>", "bool opEquals(const Handle<T>&in) const",
        asMETHOD(CppHandle, opEquals), asCALL_THISCALL));
    AS_CALL(engine->RegisterObjectMethod(
        "Handle<T>", "bool opNotEquals(const Handle<T>&in) const",
        asMETHOD(CppHandle, opNotEquals), asCALL_THISCALL));
}
