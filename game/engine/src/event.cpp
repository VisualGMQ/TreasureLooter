#include "engine/event.hpp"

#include "engine/profile.hpp"

constexpr bool NullEventListenerID::operator==(EventListenerID id) const {
    return static_cast<uint32_t>(id) == 0;
}

constexpr bool NullEventListenerID::operator!=(EventListenerID id) const {
    return !(*this == id);
}

constexpr bool NullEventListenerID::operator==(NullEventListenerID) const {
    return true;
}

constexpr bool NullEventListenerID::operator!=(NullEventListenerID) const {
    return false;
}

void EventSystem::HandleEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        EnqueueEvent(event.button);
    }
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        EnqueueEvent(event.motion);
    }
    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
        EnqueueEvent(event.key);
    }
    if (event.type == SDL_EVENT_GAMEPAD_ADDED ||
        event.type == SDL_EVENT_GAMEPAD_REMOVED) {
        EnqueueEvent(event.gdevice);
    }
    if (event.type == SDL_EVENT_WINDOW_RESIZED ||
        event.type == SDL_EVENT_WINDOW_ENTER_FULLSCREEN ||
        event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED ||
        event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
        EnqueueEvent(event.window);
    }

    // TODO: more events
}

void EventSystem::Update() {
    PROFILE_SECTION();
    
    for (auto& [_, sink] : m_sinks) {
        sink->Update();
    }
}