#pragma once
#include "SDL3/SDL.h"
#include "log.hpp"
#include "type_index.hpp"

#include <functional>
#include <memory>

enum class EventListenerID : uint32_t {};

struct NullEventListenerID {
    constexpr bool operator==(EventListenerID) const;
    constexpr bool operator!=(EventListenerID) const;
    constexpr bool operator==(NullEventListenerID) const;
    constexpr bool operator!=(NullEventListenerID) const;
};

constexpr NullEventListenerID null_event_listener_id;

template <typename T>
using EventListener = std::function<void(const T&)>;

class EventSinkBase {
public:
    virtual ~EventSinkBase() = default;
    virtual void ClearEvents() = 0;
    virtual void ClearListener() = 0;
    virtual void Update() = 0;
};

template <typename T>
class EventSink : public EventSinkBase {
public:
    using EventListener = EventListener<T>;

    void AddListener(EventListenerID id, const EventListener& listener) {
        m_listener.emplace_back({id, listener});
    }

    void RemoveListener(EventListenerID id) {
        m_listener.erase(
            std::remove_if(
                std::begin(m_listener), std::end(m_listener),
                [id](const EventListenerInfo<T>& e) { return e.m_id == id; }),
            std::end(m_listener));
    }

    void ClearEvents() { m_enqueued_events.clear(); }

    void ClearListener() { m_listener.clear(); }

    void EnqueueEvent(const T& event) { m_enqueued_events.push_back(event); }

    void TriggerEvent(const T& event) {
        for (auto& listener : m_listener) {
            listener.m_listener(event);
        }
    }

    void Update() override {
        for (auto& event : m_enqueued_events) {
            for (auto& listener : m_listener) {
                listener.m_listener(event);
            }
        }
        ClearEvents();
    }

private:
    template <typename T>
    struct EventListenerInfo {
        EventListenerID m_id;
        std::function<void(const T&)> m_listener;
    };

    std::vector<T> m_enqueued_events;
    std::vector<EventListenerInfo<T>> m_listener;
};

class EventSystem {
public:
    template <typename T>
    EventListenerID AddListener(const EventListener<T>& listener) {
        if (auto sink = ensureSink<T>()) {
            auto id = static_cast<EventListenerID>(m_cur_id++);
            sink->AddListener(id, listener);
            return id;
        }
        return {};
    }

    template <typename T>
    void RemoveListener(EventListenerID id) {
        if (auto sink = ensureSink<T>()) {
            sink->RemoveListener(id);
        }
    }

    void HandleEvent(const SDL_Event& event);

    template <typename T>
    void TriggerEvent(const T& event) {
        if (auto sink = ensureSink<T>()) {
            sink->TriggerEvent(event);
        }
    }

    template <typename T>
    void EnqueueEvent(const T& event) {
        if (auto sink = ensureSink<T>()) {
            sink->EnqueueEvent(event);
        }
    }

    void Update();

private:
    std::unordered_map<TypeIndex, std::unique_ptr<EventSinkBase>> m_sinks;
    std::underlying_type_t<EventListenerID> m_cur_id{};

    template <typename T>
    EventSink<T>* ensureSink() {
        auto type_index = TypeIndexGenerator::Get<T>();
        if (auto it = m_sinks.find(type_index); it != m_sinks.end()) {
            return static_cast<EventSink<T>*>(it->second.get());
        }
        auto result =
            m_sinks.emplace(type_index, std::make_unique<EventSink<T>>());
        if (!result.second) {
            LOGE("Failed to add event sink");
            return nullptr;
        }
        return static_cast<EventSink<T>*>(result.first->second.get());
    }
};