#pragma once
#include "pch.hpp"
#include "common.hpp"

namespace tl {

struct Event {
    enum class Type {
        Unknown = 0,
        // add your event type here
        
        _EventCount,
    } type = Type::Unknown;

    union {
        // put your events instance here 
    };
};

class EventManager {
public:
    using CallbackFn = std::function<void(const Event&)>;

    void RegistCallback(Event::Type type, const CallbackFn& callback,
                        bool callOnce = false, const std::string& name = "");
    void EnqueueEvent(const Event& event);
    void Update();

private:
    struct EventCallback {
        
        std::string name;
        CallbackFn func;
        bool callOnce = false;
    };
    
    std::vector<Event> events_;
    std::array<std::vector<EventCallback>, static_cast<size_t>(Event::Type::_EventCount)> callbacks_;
};

}