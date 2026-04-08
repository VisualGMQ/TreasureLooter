#include "engine/script/script_event_registry.hpp"

std::unordered_map<std::string, TypeIndex> ScriptEventRegistry::m_by_name;
ScriptEventRegistry ScriptEventRegistry::m_instance;

TypeIndex ScriptEventRegistry::Lookup(std::string_view name) {
    auto& m = m_instance.m_by_name;
    const std::string key{name};
    if (auto it = m.find(key); it != m.end()) {
        return it->second;
    }
    return kInvalidTypeIndex;
}
