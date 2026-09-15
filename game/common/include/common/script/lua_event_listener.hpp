#pragma once

#include "common/event.hpp"
#include "common/log.hpp"
#include "common/net/net.hpp"
#include "common/script/luabridge_include.hpp"

#include <functional>
#include <unordered_map>
#include <unordered_set>

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
            if (!result) {
                LOGE("[Lua] event callback error: {}", result.message());
            }
        };

        EventListenerID listener_id = s_es->AddListener<T>(listener);
        s_callbacks.emplace(listener_id, cb);
        s_removers.emplace(listener_id, [listener_id]() {
            s_es->RemoveListener<T>(listener_id);
        });
        return listener_id;
    }

    template <typename T>
    static EventListenerID Add(const EventListener<T>& listener) {
        EventListenerID listener_id = s_es->AddListener<T>(listener);
        s_removers.emplace(listener_id, [listener_id]() {
            s_es->RemoveListener<T>(listener_id);
        });
        return listener_id;
    }

    // NetMsg events: hand the lua callback the payload message directly
    template <typename Payload>
    static EventListenerID AddPayload(luabridge::LuaRef cb) {
        using Event = NetMsg<Payload>;

        if (!s_es || !cb.isCallable()) {
            return null_event_listener_id;
        }

        auto listener = [cb](EventListenerID id, const Event& event) {
            auto result =
                cb(static_cast<int>(id), event.m_peer, event.Payload());
            if (!result) {
                LOGE("[Lua] event callback error: {}", result.message());
            }
        };

        EventListenerID listener_id = s_es->AddListener<Event>(listener);
        s_callbacks.emplace(listener_id, cb);
        s_removers.emplace(listener_id, [listener_id]() {
            s_es->RemoveListener<Event>(listener_id);
        });
        return listener_id;
    }

    static void Remove(EventListenerID id);
    static void Clear();

private:
    static EventSystem* s_es;
    static std::unordered_map<EventListenerID, luabridge::LuaRef> s_callbacks;
    static std::unordered_map<EventListenerID, std::function<void()>>
        s_removers;
};

#define TL_BIND_LUA_EVENT_LISTENER(EventType, EventName)                  \
    .addFunction(                                                         \
        "Add" EventName, +[](EventSystem*, luabridge::LuaRef cb) -> int { \
            return static_cast<int>(                                      \
                LuaEventListenerRegistry::Add<EventType>(cb));            \
        })

#define TL_BIND_LUA_PROTO_EVENT_LISTENER(PayloadType, EventName)          \
    .addFunction(                                                         \
        "Add" EventName, +[](EventSystem*, luabridge::LuaRef cb) -> int { \
            return static_cast<int>(                                      \
                LuaEventListenerRegistry::AddPayload<PayloadType>(cb));   \
        })
