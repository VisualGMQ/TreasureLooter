#pragma once

#include "engine/flag.hpp"
#include "lua.h"
#include "lualib.h"
#include "LuaBridge/LuaBridge.h"
#include <string>
#include <type_traits>

// Intentionally no Stack<Flags<T>> specialization: use LuaBridge's default userdata-by-value
// so properties and returns are TL.*Flags objects (Has/Value/...) in Luau, not raw integers.

template <typename T>
void bindFlags(const char* name, lua_State* L) {
    static_assert(std::is_enum_v<T>, "Flags<T> only supports enum type T");
    using FlagsType = Flags<T>;
    using UnderlyingType = typename FlagsType::underlying_type;

    luabridge::getGlobalNamespace(L)
        .beginNamespace("TL")
            .beginClass<FlagsType>(name)
                .template addConstructor<void (), void (T), void (UnderlyingType)>()
                .addFunction("Value", &FlagsType::Value)
                .addFunction("Remove", &FlagsType::Remove)
                .addFunction("Has", &FlagsType::Has)
                .addFunction( "__bor", &FlagsType::operator|)
                .addFunction("__band", &FlagsType::operator&)
                .addFunction( "__bnot", &FlagsType::operator~)
                .addFunction("__tostring",
                    +[](const FlagsType& self) {
                        return std::to_string(self.Value());
                    })
            .endClass()
        .endNamespace();
}
