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

void EventManager::RemoveCallback(Event::Type type, const std::string& name) {
    pendingRemoveCallbacks_[static_cast<uint32_t>(type)].push_back(name);
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
    removePendingCallbacks();
}

void EventManager::removePendingCallbacks() {
    for (int i = 0; i < pendingClearOneType_.size(); i++) {
        if (pendingClearOneType_[i] || pendingClearAll_) {
            callbacks_[i].clear();
            pendingRemoveCallbacks_[i].clear();
        }
    }

    pendingClearAll_ = false;

    pendingClearOneType_.fill(false);

    for (int i = 0; i < static_cast<uint32_t>(Event::Type::_EventCount); i++) {
        auto& callbackNameList = pendingRemoveCallbacks_[i];
        for (auto& callbackName : callbackNameList) {
            callbacks_[i].erase(std::find_if(callbacks_[i].begin(),
                                             callbacks_[i].end(),
                                             [&](const EventCallback&
                                             callback) {
                                                 return callback.name ==
                                                     callbackName;
                                             }));
        }

        callbackNameList.clear();
    }
}
}
