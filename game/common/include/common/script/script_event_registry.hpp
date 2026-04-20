#pragma once

#include "common/type_index.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

class ScriptEventRegistry {
public:
    template <typename T>
    static void Register(std::string name) {
        m_instance.m_by_name[std::move(name)] = TypeIndexGenerator::Get<T>();
    }

    static TypeIndex Lookup(std::string_view name);

private:
    static ScriptEventRegistry m_instance;

    static std::unordered_map<std::string, TypeIndex> m_by_name;
};
