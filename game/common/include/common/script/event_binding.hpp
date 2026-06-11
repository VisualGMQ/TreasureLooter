#pragma once

#include "common/context.hpp"
#include "common/event.hpp"
#include "common/script/script.hpp"
#include "common/script/script_event_registry.hpp"

#define TL_REGISTER_EVENT_TO_SCRIPT(EventType, EventName)              \
    do {                                                               \
        ScriptEventRegistry::Register<EventType>(EventName);           \
        COMMON_CONTEXT.m_event_system->AddListener<EventType>(         \
            [](EventListenerID, const EventType& event) {              \
                COMMON_CONTEXT.m_script_component_manager->HandleEvent( \
                    event, EventName);                                 \
            });                                                        \
    } while (0)
