#include "common/script/lua_event_listener.hpp"

EventSystem* LuaEventListenerRegistry::s_es = nullptr;
std::unordered_map<int, luabridge::LuaRef>
    LuaEventListenerRegistry::s_callbacks;
std::unordered_map<int, std::function<void()>>
    LuaEventListenerRegistry::s_removers;

void LuaEventListenerRegistry::Initialize(EventSystem* es) {
    s_es = es;
}

void LuaEventListenerRegistry::Remove(EventListenerID id) {
    int key = static_cast<int>(id);
    auto it = s_removers.find(key);
    if (it != s_removers.end()) {
        it->second();
        s_removers.erase(it);
    }
    s_callbacks.erase(key);
}

void LuaEventListenerRegistry::Clear() {
    for (auto& [_, remover] : s_removers) {
        remover();
    }
    s_removers.clear();
    s_callbacks.clear();
    s_es = nullptr;
}
