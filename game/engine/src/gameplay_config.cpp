#include "engine/gameplay_config.hpp"

#include "engine/asset_manager.hpp"
#include "engine/context.hpp"
#include "engine/enemy_state.hpp"
#include "engine/profile.hpp"
#include "engine/relationship.hpp"

void VirtualButton::Update() {
    if (!m_button) {
        return;
    }

    if (m_button->IsPressing()) {
        GAME_CONTEXT.m_input_manager->AcceptFingerButton(
            m_action_name, Action::State::Pressing);
    }

    if (m_button->IsReleasing()) {
        GAME_CONTEXT.m_input_manager->AcceptFingerButton(
            m_action_name, Action::State::Releasing);
        m_button = nullptr;
    }
}