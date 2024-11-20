#include "event.hpp"

#include "macro.hpp"

namespace tl {
void EventManager::RegistCallback(Event::Type type, const CallbackFn& callback,
                                  bool callOnce, const std::string& name) {
    EventCallback cb;
    cb.callOnce = callOnce;
    cb.name = name;
    cb.func = callback;

    callbacks_[static_cast<uint32_t>(type)].push_back(cb);
}

void EventManager::RemoveCallback(Event::Type type, std::string_view name) {
    auto& callbackList = callbacks_[static_cast<uint32_t>(type)];
    callbackList.erase(std::find(callbackList.begin(), callbackList.end(),
                                 [=](const EventCallback& callback) {
                                     return callback.name == name;
                                 }));
}

void EventManager::RemoveAllCallbackIn(Event::Type type) {
    callbacks_[static_cast<uint32_t>(type)].clear();
}

void EventManager::RemoveAllCallback() {
    for (auto& callback : callbacks_) {
        callback.clear();
    }
}

void EventManager::EnqueueEnterTriggerAreaEvent(
    GameObject* go, const MarkedActor& area) {
    Event event;
    event.type = Event::Type::EnterTriggerArea;
    event.enterTriggerArea.go = go;
    event.enterTriggerArea.area = area;
    events_.emplace_back(std::move(event));
}

void EventManager::EnqueueLeaveTriggerAreaEvent(GameObject* go,
                                                const MarkedActor& area) {
    Event event;
    event.type = Event::Type::LeaveTriggerArea;
    event.leaveTriggerArea.go = go;
    event.leaveTriggerArea.area = area;
    events_.emplace_back(std::move(event));
}

void EventManager::Update() {
    for (auto& event : events_) {
        TL_CONTINUE_IF_FALSE(event.type != Event::Type::Unknown);
        std::vector<size_t> needRemoveCallbacks;

        auto& callbacks = callbacks_[static_cast<uint32_t>(event.type)];
        for (size_t i = 0; i < callbacks.size(); i++) {
            EventCallback& callback = callbacks[i];

            callback.func(event);
            if (callback.callOnce) {
                needRemoveCallbacks.push_back(i);
            }
        }

        for (auto idx : needRemoveCallbacks) {
            callbacks.erase(callbacks.begin() + idx);
        }
    }

    events_.clear();
}
}
