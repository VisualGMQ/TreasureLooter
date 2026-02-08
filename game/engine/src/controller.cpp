#include "engine/controller.hpp"
#include "engine/asset_manager.hpp"
#include "engine/input/mouse.hpp"
#include "engine/level.hpp"
#include "engine/relationship.hpp"

ControllerAxis::ControllerAxis(SDL_JoystickID joystick_id, const Axis& axis)
    : m_joystick_id{joystick_id}, m_axis{axis} {}

float ControllerAxis::Value() const {
    return m_axis.Value(m_joystick_id);
}

ControllerAxises::ControllerAxises(SDL_JoystickID joystick_id,
                                   const Axises& axises)
    : m_joystick_id{joystick_id}, m_axises{axises} {}

Vec2 ControllerAxises::Value() const {
    return m_axises.Value(m_joystick_id);
}

ControllerAction::ControllerAction(SDL_JoystickID joystick_id,
                                   const Action& action)
    : m_joystick_id{joystick_id}, m_action{action} {}

bool ControllerAction::IsPressed() const {
    return m_action.IsPressed(m_joystick_id);
}

bool ControllerAction::IsPressing() const {
    return m_action.IsPressing(m_joystick_id);
}

bool ControllerAction::IsReleased() const {
    return m_action.IsReleased(m_joystick_id);
}

bool ControllerAction::IsReleasing() const {
    return m_action.IsReleasing(m_joystick_id);
}

bool ControllerAction::IsRelease() const {
    return m_action.IsRelease(m_joystick_id);
}

bool ControllerAction::IsPress() const {
    return m_action.IsPress(m_joystick_id);
}

PlayerController::PlayerController(InputManager& input_mgr,
                                   EventSystem& event_system,
                                   AssetsManager& assets_mgr,
                                   TransformManager& transform_mgr,
                                   RelationshipManager& relationship_mgr)
    : m_input_mgr{input_mgr},
      m_event_system{event_system},
      m_assets_mgr{assets_mgr},
      m_transform_mgr{transform_mgr},
      m_relationship_mgr{relationship_mgr} {
    m_event_system.AddListener<SDL_GamepadDeviceEvent>(
        [&](EventListenerID, const SDL_GamepadDeviceEvent& event) {
            if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
                if (m_joystick_id == 0) {
                    m_joystick_id = event.which;
                }
            } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
                if (m_joystick_id == event.which) {
                    m_joystick_id = 0;
                }
            }
        });
}

PlayerController::~PlayerController() {
    m_event_system.RemoveListener<SDL_GamepadDeviceEvent>(m_gamepad_event);
    m_event_system.RemoveListener<SDL_WindowEvent>(m_window_resize_event_listener);
}

ControllerAxis PlayerController::GetAxis(const std::string& name) const {
    return ControllerAxis(m_joystick_id, m_input_mgr.GetAxis(name));
}

ControllerAction PlayerController::GetAction(const std::string& name) const {
    return ControllerAction(m_joystick_id, m_input_mgr.GetAction(name));
}

ControllerAxises PlayerController::MakeAxises(const std::string& x_name,
                                              const std::string& y_name) {
    return ControllerAxises(m_joystick_id,
                            m_input_mgr.MakeAxises(x_name, y_name));
}

void PlayerController::RegisterVirtualController(LevelHandle level, const GameConfig& game_config) {
#ifdef TL_ANDROID
    if (level) {
        initVirualJoystick(level);
        initVirualAttackButton(level, game_config);

        // NOTE: when under android, device will change window size after few
        // frames and send multiple SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED event
        // after rotate screen. So we must listen this event and change our
        // button position
        m_window_resize_event_listener =
            m_event_system.AddListener<SDL_WindowEvent>(
                [&](EventListenerID, const SDL_WindowEvent& event) {
                    if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                        int h = event.data2;
                        m_virtual_joystick.m_circle.m_center.x =
                            game_config.m_virtual_joystick.m_offset.x;
                        m_virtual_joystick.m_circle.m_center.y =
                            h - game_config.m_virtual_joystick.m_offset.y;
                    }
                });
    }
#endif
}

void PlayerController::DestroyVirtualController(LevelHandle level) {
#ifdef TL_ANDROID
    if (level) {
        level->RemoveEntity(m_virtual_joystick_entity);
    }
#endif
}

void PlayerController::initVirualJoystick(LevelHandle level) {
    PrefabHandle joystick_prefab = m_assets_mgr.GetManager<Prefab>().Load(
        "assets/gpa/ui/android_joystick.prefab.xml");
    Transform transform;
    transform.m_size.w = m_virtual_joystick.m_circle.m_radius * 2.0;
    transform.m_size.h = m_virtual_joystick.m_circle.m_radius * 2.0;
    transform.m_position = m_virtual_joystick.m_circle.m_center -
                           Vec2{m_virtual_joystick.m_circle.m_radius,
                                m_virtual_joystick.m_circle.m_radius};
    m_virtual_joystick_entity = level->Instantiate(joystick_prefab);
    *GAME_CONTEXT.m_transform_manager->Get(m_virtual_joystick_entity) =
        transform;

    Relationship* ui_relationship =
        m_relationship_mgr.Get(level->GetUIRootEntity());
    ui_relationship->m_children.push_back(m_virtual_joystick_entity);
    m_virtual_joystick.m_ui_entity = m_virtual_joystick_entity;

    m_event_system.AddListener<UIDragEvent>(
        std::bind(&PlayerController::handleJoystickDragEvent, this,
                  std::placeholders::_1, std::placeholders::_2));
    m_event_system.AddListener<UIMouseUpEvent>(
        std::bind(&PlayerController::handleJoystickReleaseEvent, this,
                  std::placeholders::_1, std::placeholders::_2));
}

void PlayerController::initVirualAttackButton(LevelHandle level,
                                              const GameConfig& game_config) {
    PrefabHandle button_prefab = m_assets_mgr.GetManager<Prefab>().Load(
        "assets/gpa/ui/attack_button.prefab.xml");
    Transform transform;
    transform.m_size.w = game_config.m_virtual_joystick.m_radius * 2.0;
    transform.m_size.h = game_config.m_virtual_joystick.m_radius * 2.0;
    transform.m_position = GAME_CONTEXT.m_window->GetWindowSize() -
                           game_config.m_virtual_attack_button.m_offset;
    Entity entity = level->Instantiate(button_prefab);
    *m_transform_mgr.Get(entity) = transform;

    Relationship* ui_relationship =
        m_relationship_mgr.Get(level->GetUIRootEntity());
    ui_relationship->m_children.push_back(entity);
    m_virtual_attack_button.m_ui_entity = entity;
    m_virtual_attack_button.m_action_name = "Attack";

    m_event_system.AddListener<UIMouseDownEvent>(
        std::bind(&PlayerController::handleVirtualAttackButtonPressedEvent,
                  this, std::placeholders::_1, std::placeholders::_2));
    m_event_system.AddListener<UIMouseUpEvent>(
        std::bind(&PlayerController::handleVirtualAttackButtonReleasedEvent,
                  this, std::placeholders::_1, std::placeholders::_2));
}

void PlayerController::handleJoystickDragEvent(EventListenerID id,
                                               const UIDragEvent& event) {
    if (event.m_entity != m_virtual_joystick.m_ui_entity) {
        return;
    }

    Transform* transform =
        GAME_CONTEXT.m_transform_manager->Get(event.m_entity);
    Vec2 position = GAME_CONTEXT.m_mouse->Position();
    Vec2 dir = position - m_virtual_joystick.m_circle.m_center;
    float dir_len = dir.Length();
    Vec2 offset = Vec2::ZERO;
    float offset_len = 0;
    if (!FLT_EQ(dir_len, 0)) {
        dir /= dir_len;
        offset = dir;
    }
    offset_len = std::min(dir_len, m_virtual_joystick.m_max_drag_dist);
    transform->m_position = m_virtual_joystick.m_circle.m_center +
                            offset * offset_len -
                            Vec2{m_virtual_joystick.m_circle.m_radius,
                                 m_virtual_joystick.m_circle.m_radius};
    m_input_mgr.AcceptFingerAxisEvent(
        "MoveX", dir.x * offset_len / m_virtual_joystick.m_max_drag_dist);
    m_input_mgr.AcceptFingerAxisEvent(
        "MoveY", dir.y * offset_len / m_virtual_joystick.m_max_drag_dist);
}

void PlayerController::handleJoystickReleaseEvent(EventListenerID id,
                                                  const UIMouseUpEvent& event) {
    if (event.m_entity != m_virtual_joystick.m_ui_entity) {
        return;
    }

    Transform* transform = m_transform_mgr.Get(event.m_entity);
    transform->m_position = m_virtual_joystick.m_circle.m_center -
                            Vec2{m_virtual_joystick.m_circle.m_radius,
                                 m_virtual_joystick.m_circle.m_radius};
    m_input_mgr.AcceptFingerAxisEvent("MoveX", 0);
    m_input_mgr.AcceptFingerAxisEvent("MoveY", 0);
}

void PlayerController::handleVirtualAttackButtonPressedEvent(
    EventListenerID id, const UIMouseDownEvent& event) {
    if (event.m_entity != m_virtual_attack_button.m_ui_entity) {
        return;
    }

    m_virtual_attack_button.m_button = &event.m_button;
    m_input_mgr.AcceptFingerButton(m_virtual_attack_button.m_action_name,
                                   Action::State::Pressed);
}

void PlayerController::handleVirtualAttackButtonReleasedEvent(
    EventListenerID id, const UIMouseUpEvent& event) {
    if (event.m_entity != m_virtual_attack_button.m_ui_entity) {
        return;
    }

    m_virtual_attack_button.m_button = &event.m_button;
    m_input_mgr.AcceptFingerButton(m_virtual_attack_button.m_action_name,
                                   Action::State::Released);
}

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
