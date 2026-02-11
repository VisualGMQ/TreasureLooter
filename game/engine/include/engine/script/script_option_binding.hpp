#pragma once
#include "angelscript.h"
#include "autowrapper/aswrappedcall.h"
#include "engine/script/script_macros.hpp"
#include <optional>
#include <string>

class CppOptional {
public:
    explicit CppOptional(asITypeInfo* type_info);
    CppOptional(const CppOptional& other);
    ~CppOptional();
    void SetValue(void* value);
    bool HasValue() const;
    void* Value() const;
    void Reset();

private:
    void destructValueType(asIScriptEngine* engine, void* ptr,
                          asITypeInfo* subtype);

    union {
        void* m_value;
        uint64_t _ = {0};  // make sure m_value can be deref as uint64_t;
    };

    asITypeInfo* m_type_info = nullptr;
    size_t m_elem_size = 0;
    bool m_has_value = false;   // needed to distinguish empty from Optional<T>(0)
    bool m_owns_buffer = false;  // true when m_value points to heap (object value type)
};

// Helpers for Optional<T> construct/destruct (implemented in .cpp)
void optionalDefaultConstruct(asITypeInfo* ti, void* obj);
void optionalConstructWithValue(asITypeInfo* ti, void* value, void* obj);
void optionalDestruct(void* obj);

void registerOptionalType(asIScriptEngine* engine);

void bindOptionalType(asIScriptEngine* engine);

// Convert between std::optional<T> and CppOptional (for schema-generated bindings).
template <typename T>
CppOptional OptionalFromStdOptional(asIScriptEngine* engine,
                                    const std::optional<T>& opt,
                                    const std::string& subtype_name) {
    std::string final_subtype_name = std::is_class_v<T> || std::is_enum_v<T>
                             ? "TL::" + subtype_name
                             : subtype_name;
    std::string decl = "TL::Optional<" + final_subtype_name + ">";
    asITypeInfo* ti = engine->GetTypeInfoByDecl(decl.c_str());
    CppOptional cpp_opt(ti);
    if (opt.has_value())
        cpp_opt.SetValue(const_cast<T*>(&opt.value()));
    return cpp_opt;
}

template <typename T>
void StdOptionalFromOptional(const CppOptional* o, std::optional<T>& out) {
    if (o->HasValue())
        out = *reinterpret_cast<const T*>(o->Value());
    else
        out = std::nullopt;
}
