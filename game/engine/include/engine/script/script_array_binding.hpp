#pragma once
#include "angelscript.h"
#include "scriptarray/scriptarray.h"
#include <string>
#include <type_traits>
#include <vector>

template <typename T>
inline CScriptArray* VectorToScriptArray(asIScriptEngine* engine,
                                        const std::vector<T>& vec,
                                        const std::string& subtype_name) {
    std::string final_subtype_name = std::is_class_v<T> || std::is_enum_v<T>
        ? "TL::" + subtype_name : subtype_name;                                  
    std::string decl = "array<" + final_subtype_name + ">";
    asITypeInfo* ti = engine->GetTypeInfoByDecl(decl.c_str());
    if (!ti)
        return nullptr;
    CScriptArray* arr = CScriptArray::Create(ti, static_cast<asUINT>(vec.size()));
    if (!arr)
        return nullptr;
    for (size_t i = 0; i < vec.size(); ++i)
        arr->SetValue(static_cast<asUINT>(i), (T*)(&vec[i]));
    return arr;
}

template <typename T>
inline void ScriptArrayToVector(asIScriptEngine*,
                                CScriptArray* arr,
                                std::vector<T>& out) {
    if (!arr) {
        out.clear();
        return;
    }
    const asUINT n = arr->GetSize();
    out.resize(static_cast<size_t>(n));
    for (asUINT i = 0; i < n; ++i)
        out[i] = *static_cast<const T*>(arr->At(i));
}
