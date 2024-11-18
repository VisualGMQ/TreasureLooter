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

void EventManager::EnqueueEvent(const Event& event) {
    events_.push_back(event);
}

void EventManager::Update() {
    for (auto& event : events_) {
        TL_CONTINUE_IF(event.type != Event::Type::Unknown);
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
