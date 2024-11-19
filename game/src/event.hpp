#pragma once
#include "pch.hpp"
#include "common.hpp"
#include "physics_scene.hpp"

namespace tl {

struct PhysicsAreaTriggerEvent {
    MarkedActor src;
    MarkedActor dst;
};

struct Event {
    enum class Type {
        Unknown = 0,
        PhysicsAreaTigger,
        // add your event type here
        
        _EventCount,
    } type = Type::Unknown;

    union {
        PhysicsAreaTriggerEvent physicsAreaTrigger{};
    };
};

class EventManager {
public:
    using CallbackFn = std::function<void(const Event&)>;

    void RegistCallback(Event::Type type, const CallbackFn& callback,
                        bool callOnce = false, const std::string& name = "");
    void EnqueuPhysicsAreaTriggerEvent(const MarkedActor& src, const MarkedActor& dst);
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