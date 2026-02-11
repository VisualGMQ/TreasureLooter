#pragma once
#include "angelscript.h"
#include "engine/handle.hpp"
#include "engine/script/script_macros.hpp"

// Storage for a copy of Handle<T> (same size for all T).
constexpr size_t kCppHandleStorageSize = sizeof(Handle<int>);

class CppHandle {
public:
    explicit CppHandle(asITypeInfo* type_info);
    CppHandle(asITypeInfo* type_info, void* handle_ptr);
    CppHandle(const CppHandle& other);
    ~CppHandle();
    void* Get() const;  // returns T* (first member of Handle<T>)
    void* GetHandlePtr() const;  // returns address of Handle<T> (for copy)
    CppHandle& opAssign(const CppHandle& other);
    bool opEquals(const CppHandle& other) const;
    bool opNotEquals(const CppHandle& other) const;

    // Set from a copy of Handle<T> (for return values / set from script).
    template <typename T>
    void SetFromCopy(const Handle<T>& h) {
        *(Handle<T>*)(m_storage) = h;
    }

private:
    asITypeInfo* m_type_info = nullptr;
    alignas(void*) char m_storage[kCppHandleStorageSize];
};

void handleDefaultConstruct(asITypeInfo* ti, void* obj);
void handleConstructFromPtr(asITypeInfo* ti, void* handle_ptr, void* obj);
void handleDestruct(void* obj);

void registerHandleType(asIScriptEngine* engine);
void bindHandleType(asIScriptEngine* engine);

template <typename T>
void bindHandleTypeOpImplCast(asIScriptEngine* engine,
                              const char* handle_type_decl,
                              const char* underlying_type_name) {
    if (!engine->GetTypeInfoByDecl(handle_type_decl))
        return;
    std::string sig = std::string(underlying_type_name) + "@ opImplCast()";
    AS_CALL(engine->RegisterObjectMethod(
        handle_type_decl, sig.c_str(),
        asFUNCTION(+[](CppHandle* h) -> T* {
            void* p = h->Get();
            return static_cast<T*>(p);
        }),
        asCALL_CDECL_OBJLAST));
}

template <typename T>
CppHandle CppHandleFromHandle(asIScriptEngine* engine,
                              const Handle<T>& h) {
    asITypeInfo* ti = engine->GetTypeInfoByName("TL::Handle");
    CppHandle cpp(ti);
    cpp.SetFromCopy(h);
    return cpp;
}

template <typename T>
void HandleFromCppHandle(const CppHandle* cpp_handle, Handle<T>& out) {
    if (!cpp_handle || !cpp_handle->GetHandlePtr())
        out = Handle<T>();
    else
        out = *static_cast<const Handle<T>*>(cpp_handle->GetHandlePtr());
}
