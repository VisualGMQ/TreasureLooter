#pragma once
#include "angelscript.h"
#include "scriptarray/scriptarray.h"
#include <vector>

// Convert between std::vector<T> and AngelScript CScriptArray (array<T>@).
// Used by schema-generated getters/setters and engine bindings.
// Always returns a new array with copied data (never a reference to C++ storage).

template <typename T>
inline CScriptArray* VectorToScriptArray(asIScriptEngine* engine,
                                        const std::vector<T>& vec,
                                        const char* array_type_decl) {
    asITypeInfo* ti = engine->GetTypeInfoByDecl(array_type_decl);
    if (!ti)
        return nullptr;
    CScriptArray* arr = CScriptArray::Create(ti, static_cast<asUINT>(vec.size()));
    if (!arr)
        return nullptr;
    for (size_t i = 0; i < vec.size(); ++i)
        arr->SetValue(static_cast<asUINT>(i), const_cast<T*>(&vec[i]));
    return arr;
}

template <typename T>
inline void ScriptArrayToVector(asIScriptEngine* /*engine*/,
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
