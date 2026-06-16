#pragma once

#include "common/event.hpp"
#include "common/log.hpp"
#include "common/script/luabridge_include.hpp"

#include <functional>
#include <unordered_map>

class LuaEventListenerRegistry {
public:
    static void Initialize(EventSystem* es);

    template <typename T>
    static EventListenerID Add(luabridge::LuaRef cb) {
        if (!s_es || !cb.isCallable()) {
            return null_event_listener_id;
        }

        auto listener = [cb](EventListenerID id, const T& event) {
            auto result = cb(static_cast<int>(id), event);
            if (result.errorCode()) {
                LOGE("[Lua] event callback error: {}", result.errorMessage());
            }
        };

        EventListenerID eid = s_es->AddListener<T>(listener);
        int key = static_cast<int>(eid);
        s_callbacks.emplace(key, cb);
        s_removers.emplace(key, [eid]() { s_es->RemoveListener<T>(eid); });
        return eid;
    }

    static void Remove(EventListenerID id);
    static void Clear();

private:
    static EventSystem* s_es;
    static std::unordered_map<int, luabridge::LuaRef> s_callbacks;
    static std::unordered_map<int, std::function<void()>> s_removers;
};

#define TL_BIND_LUA_EVENT_LISTENER(EventType, EventName)             \
    .addFunction(                                                    \
        "Add" EventName,                                             \
        +[](EventSystem*, luabridge::LuaRef cb) -> int {             \
            return static_cast<int>(                                  \
                LuaEventListenerRegistry::Add<EventType>(cb));        \
        })
