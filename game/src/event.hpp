#pragma once
#include "pch.hpp"
#include "common.hpp"
#include "physics_scene.hpp"

namespace tl {

struct EnterTriggerArea {
    GameObject* go = nullptr;
    MarkedActor area;
};

struct LeaveTriggerArea {
    GameObject* go = nullptr;
    MarkedActor area;
};


struct Event {
    enum class Type {
        Unknown = 0,
        EnterTriggerArea,
        LeaveTriggerArea,
        // add your event type here
        
        _EventCount,
    } type = Type::Unknown;

    union {
        EnterTriggerArea enterTriggerArea;
        LeaveTriggerArea leaveTriggerArea;
    };

    Event() : enterTriggerArea{} { }
};

class EventManager {
public:
    using CallbackFn = std::function<void(const Event&)>;

    void RegistCallback(Event::Type type, const CallbackFn& callback,
                        bool callOnce = false, const std::string& name = "");
    void RemoveCallback(Event::Type type, std::string_view name);
    void RemoveAllCallbackIn(Event::Type type);
    void RemoveAllCallback();
    void EnqueueEnterTriggerAreaEvent(GameObject*, const MarkedActor& dst);
    void EnqueueLeaveTriggerAreaEvent(GameObject*, const MarkedActor& dst);
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