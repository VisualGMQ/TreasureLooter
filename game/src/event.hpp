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

struct CollisionEvent {
    MarkedActor src;
    MarkedActor dst;
};



struct Event {
    enum class Type {
        Unknown = 0,
        EnterTriggerArea,
        LeaveTriggerArea,

        Collision,
        // add your event type here
        
        _EventCount,
    } type = Type::Unknown;

    union {
        EnterTriggerArea enterTriggerArea;
        LeaveTriggerArea leaveTriggerArea;
        CollisionEvent collision;
    };

    Event() : enterTriggerArea{} { }
};

class EventManager {
public:
    using CallbackFn = std::function<void(const Event&)>;

    EventManager();

    void RegistCallback(Event::Type type, const CallbackFn& callback,
                        bool callOnce = false, const std::string& name = "");
    void RemoveCallback(Event::Type type, const std::string& name);
    void RemoveAllCallbackIn(Event::Type type);
    void RemoveAllCallback();
    void EnqueueEnterTriggerAreaEvent(GameObject*, const MarkedActor& dst);
    void EnqueueLeaveTriggerAreaEvent(GameObject*, const MarkedActor& dst);
    void EnqueueCollisionEvent(MarkedActor, MarkedActor dst);

    void Update();

private:
    struct EventCallback {
        std::string name;
        CallbackFn func;
        bool callOnce = false;
    };
    
    static constexpr size_t EventCount = static_cast<size_t>(Event::Type::_EventCount);
    std::vector<Event> events_;
    std::array<std::vector<EventCallback>, EventCount> callbacks_;
    std::array<std::vector<std::string>, EventCount> pendingRemoveCallbacks_;
    std::array<bool, EventCount> pendingClearOneType_;
    bool pendingClearAll_ = false;

    void removePendingCallbacks();
};

}