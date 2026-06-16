#include "client/ui.hpp"

#include "SDL3/SDL_keyboard.h"
#include "client/asset_manager.hpp"
#include "client/context.hpp"
#include "client/draw_order.hpp"
#include "client/input/finger_touch.hpp"
#include "client/input/mouse.hpp"
#include "client/window.hpp"
#include "common/macros.hpp"
#include "common/profile.hpp"
#include "common/relationship.hpp"

void UITextInput::SetFont(FontHandle font) {
    m_font = font;
    regenerateText();
}

void UITextInput::SetFontPt(uint32_t pt) {
    m_pt = pt;
    regenerateText();
}

const UTF8String& UITextInput::GetText() const {
    return m_text;
}

void UITextInput::SetText(const std::string& text) {
    m_text = UTF8String(text);
    m_cursor_pos = m_text.size();
    m_cursor_x_dirty = true;
    regenerateText();
}

void UITextInput::SetText(const UTF8String& text) {
    m_text = text;
    m_cursor_pos = text.size();
    m_cursor_x_dirty = true;
    regenerateText();
}

size_t UITextInput::GetCursorPos() const {
    return m_cursor_pos;
}

void UITextInput::SetCursorPos(size_t pos) {
    if (pos > m_text.size()) pos = m_text.size();
    m_cursor_pos = pos;
    m_cursor_x_dirty = true;
}

void UITextInput::MoveCursorLeft() {
    if (m_cursor_pos > 0) {
        m_cursor_pos--;
        m_cursor_x_dirty = true;
    }
}

void UITextInput::MoveCursorRight() {
    if (m_cursor_pos < m_text.size()) {
        m_cursor_pos++;
        m_cursor_x_dirty = true;
    }
}

void UITextInput::MoveCursorHome() {
    m_cursor_pos = 0;
    m_cursor_x_dirty = true;
}

void UITextInput::MoveCursorEnd() {
    m_cursor_pos = m_text.size();
    m_cursor_x_dirty = true;
}

void UITextInput::DeleteBeforeCursor() {
    TL_RETURN_IF_TRUE(m_cursor_pos == 0);
    m_text.erase(m_cursor_pos - 1, 1);
    m_cursor_pos--;
    m_cursor_x_dirty = true;
    regenerateText();
}

void UITextInput::DeleteAfterCursor() {
    TL_RETURN_IF_TRUE(m_cursor_pos >= m_text.size());
    m_text.erase(m_cursor_pos, 1);
    m_cursor_x_dirty = true;
    regenerateText();
}

Image& UITextInput::GetTextImage() {
    return m_text_image;
}

Vec2 UITextInput::GetTextImageSize() const {
    return m_text_image.GetSize();
}

void UITextInput::RefreshText() {
    regenerateText();
}

float UITextInput::GetCursorX() {
    TL_RETURN_VALUE_IF_FALSE(m_cursor_pos != 0, 0);

    if (m_cursor_x_dirty) {
        m_cursor_x_dirty = false;
        m_font->SetFontSize(m_pt);
        UTF8String left = m_text.substr(0, m_cursor_pos);
        std::string left_utf8(left.c_str());
        SDL_Surface* surf = m_font->GenerateText(left_utf8, m_color);
        m_cursor_x_cache = surf ? static_cast<float>(surf->w) : 0;
        if (surf) {
            SDL_DestroySurface(surf);
        }
    }
    return m_cursor_x_cache;
}

void UITextInput::regenerateText() {
    m_font->SetFontSize(m_pt);
    m_cursor_x_dirty = true;
    TL_RETURN_IF_TRUE(m_text.empty());
    m_text_image =
        Image{*CLIENT_CONTEXT.m_renderer,
              m_font->GenerateText(std::string(m_text.c_str()), m_color)};
}

void UITextInput::HandleTextInput(const SDL_TextInputEvent& event) {
    UTF8String input(event.text);
    bool changed = false;
    for (auto it = input.begin(); it != input.end(); ++it) {
        char32_t cp = *it;
        if (cp == U'\b' || cp == U'\r' || cp == U'\n' || cp == U'\t') continue;
        m_text.insert(m_cursor_pos, cp);
        m_cursor_pos++;
        changed = true;
    }
    if (changed) {
        m_cursor_x_dirty = true;
        regenerateText();
    }
}

void UIText::SetFont(FontHandle font) {
    m_font = font;
    regenerateText();
}

void UIText::ChangeText(const std::string& text) {
    m_text = text;
    regenerateText();
}

void UIText::ChangeTextPt(uint32_t pt) {
    m_pt = pt;
    regenerateText();
}

Vec2 UIText::GetTextImageSize() const {
    return m_text_image.GetSize();
}

const std::string& UIText::GetText() const {
    return m_text;
}

Image& UIText::GetTextImage() {
    return m_text_image;
}

void UIText::regenerateText() {
    if (m_text.empty()) {
        return;
    }

    m_font->SetFontSize(m_pt);

    m_text_image = Image{*CLIENT_CONTEXT.m_renderer,
                         m_font->GenerateText(m_text, m_color)};
}

void UIPanelComponent::UpdateSize(const Transform& old_transform,
                                  const Transform& new_transform,
                                  const Relationship& relationship,
                                  const UIWidget& ui, bool force) {
    if (!force && old_transform.m_size == new_transform.m_size) {
        return;
    }

    for (size_t i = 0; i < relationship.GetChildrenCount(); i++) {
        Entity child = relationship.Get(i);
        Transform* transform = CLIENT_CONTEXT.m_transform_manager->Get(child);
        UIWidget* child_ui = CLIENT_CONTEXT.m_ui_manager->Get(child);

        if (!transform || !child_ui) {
            continue;
        }

        if (child_ui->m_anchor == UIAnchor::None ||
            child_ui->m_anchor == UIAnchor::Center) {
            continue;
        }

        if (child_ui->m_anchor & UIAnchor::Left &&
            child_ui->m_anchor & UIAnchor::Right) {
            assert(new_transform.m_size.w != 0);
            float left = transform->m_position.x - old_transform.m_position.x;
            float right = old_transform.m_position.x + old_transform.m_size.w -
                          (transform->m_position.x + transform->m_size.w);
            transform->m_size.w = new_transform.m_size.w - left - right;
        }

        if (child_ui->m_anchor & UIAnchor::Top &&
            child_ui->m_anchor & UIAnchor::Bottom) {
            assert(new_transform.m_size.h != 0);
            float top = transform->m_position.y - old_transform.m_position.y;
            float bottom = old_transform.m_position.y + old_transform.m_size.h -
                           (transform->m_position.y + transform->m_size.h);
            transform->m_size.h = new_transform.m_size.h - top - bottom;
        }
    }
}

void UIPanelComponent::UpdatePosition(const Transform& old_transform,
                                      const Transform& new_transform,
                                      const Relationship& relationship,
                                      const UIWidget& ui, bool force) {
    if (!force && old_transform.m_size == new_transform.m_size) {
        return;
    }

    for (size_t i = 0; i < relationship.GetChildrenCount(); i++) {
        Entity child = relationship.Get(i);
        Transform* child_transform =
            CLIENT_CONTEXT.m_transform_manager->Get(child);
        UIWidget* child_ui = CLIENT_CONTEXT.m_ui_manager->Get(child);

        if (!child_transform || !child_ui) {
            continue;
        }

        if (child_ui->m_anchor == UIAnchor::None) {
            continue;
        }

        if (child_ui->m_anchor == UIAnchor::Center) {
            Vec2 offset =
                child_transform->m_position -
                (old_transform.m_position + old_transform.m_size * 0.5);
            child_transform->m_position =
                new_transform.m_position + new_transform.m_size * 0.5 + offset;
            continue;
        }

        if (child_ui->m_anchor & UIAnchor::Left &&
            child_ui->m_anchor & UIAnchor::Right) {
            float offset =
                child_transform->m_position.x - old_transform.m_position.x;
            child_transform->m_position.x = new_transform.m_position.x + offset;
        } else if (child_ui->m_anchor & UIAnchor::Left) {
            float offset =
                child_transform->m_position.x - old_transform.m_position.x;
            child_transform->m_position.x = new_transform.m_position.x + offset;
        } else if (child_ui->m_anchor & UIAnchor::Right) {
            float offset =
                child_transform->m_position.x -
                (old_transform.m_position.x + old_transform.m_size.w);
            child_transform->m_position.x =
                new_transform.m_position.x + new_transform.m_size.w + offset;
        }

        if (child_ui->m_anchor & UIAnchor::Top &&
            child_ui->m_anchor & UIAnchor::Bottom) {
            float offset =
                child_transform->m_position.y - old_transform.m_position.y;
            child_transform->m_position.y = new_transform.m_position.y + offset;
        } else if (child_ui->m_anchor & UIAnchor::Top) {
            float offset =
                child_transform->m_position.y - old_transform.m_position.y;
            child_transform->m_position.y = new_transform.m_position.y + offset;
        } else if (child_ui->m_anchor & UIAnchor::Bottom) {
            float offset =
                child_transform->m_position.y -
                (old_transform.m_position.y + old_transform.m_size.h);
            child_transform->m_position.y =
                new_transform.m_position.y + new_transform.m_size.h + offset;
        }
    }
}

void UIBoxPanelComponent::UpdateSize(const Transform& old_transform,
                                     const Transform& new_transform,
                                     const Relationship& relationship,
                                     const UIWidget& ui, bool force) {
    if (!force && old_transform == new_transform) {
        return;
    }

    Vec2 ceil_size;
    if (!relationship.HasChildren()) {
        ceil_size = new_transform.m_size - ui.m_padding * 2.0;
        float totle_spacing =
            m_spacing * std::max((int)relationship.GetChildrenCount() - 1, 0);
        if (m_type == UIBoxPanelType::Vertical) {
            ceil_size.h -= totle_spacing;
        } else {
            ceil_size.w -= totle_spacing;
        }
        ceil_size -= ui.m_margin * 2.0;

        if (m_type == UIBoxPanelType::Vertical) {
            ceil_size.h /= relationship.GetChildrenCount();
        } else {
            ceil_size.w /= relationship.GetChildrenCount();
        }
    }

    for (size_t i = 0; i < relationship.GetChildrenCount(); i++) {
        Entity entity = relationship.Get(i);
        UIWidget* ui = CLIENT_CONTEXT.m_ui_manager->Get(entity);
        Transform* child_transform =
            CLIENT_CONTEXT.m_transform_manager->Get(entity);
        if (!child_transform || !ui) {
            continue;
        }

        child_transform->m_size = ceil_size;
    }
}

void UIBoxPanelComponent::UpdatePosition(const Transform& old_transform,
                                         const Transform& new_transform,
                                         const Relationship& relationship,
                                         const UIWidget& ui, bool force) {
    if (!force && old_transform == new_transform) {
        return;
    }

    Vec2 start_position = new_transform.m_position + ui.m_padding;

    for (size_t i = 0; i < relationship.GetChildrenCount(); i++) {
        Entity entity = relationship.Get(i);
        UIWidget* ui = CLIENT_CONTEXT.m_ui_manager->Get(entity);
        Transform* child_transform =
            CLIENT_CONTEXT.m_transform_manager->Get(entity);
        if (!child_transform || !ui) {
            continue;
        }

        if (m_type == UIBoxPanelType::Vertical) {
            child_transform->m_position =
                start_position +
                Vec2{0, child_transform->m_size.h + m_spacing} * i;
        } else {
            child_transform->m_position =
                start_position +
                Vec2{child_transform->m_size.w + m_spacing, 0} * i;
        }
        child_transform->m_position += ui->m_margin * 2.0;
    }
}

UIWidget::UIWidget() {
    m_old_transform.m_size = Vec2::ZERO;
}

UIWidget::UIWidget(UIWidgetDefinitionHandle info) {
    m_old_transform.m_size = Vec2::ZERO;

    m_enable_draw = info->m_enable_draw;
    m_selected = info->m_selected;
    m_disabled = info->m_disabled;
    m_anchor = info->m_anchor;
    m_margin = info->m_margin;
    m_use_clip = info->m_use_clip;
    m_normal_theme = info->m_normal_theme;
    m_can_be_selected = info->m_can_be_selected;

    if (info->m_hover_theme) {
        m_hover_theme = std::make_unique<UITheme>(info->m_hover_theme.value());
    }
    if (info->m_down_theme) {
        m_down_theme = std::make_unique<UITheme>(info->m_down_theme.value());
    }
    if (info->m_disabled_theme) {
        m_disabled_theme =
            std::make_unique<UITheme>(info->m_disabled_theme.value());
    }
    if (info->m_selected_theme) {
        m_selected_theme =
            std::make_unique<UITheme>(info->m_selected_theme.value());
    }
    if (info->m_selected_disabled_theme) {
        m_selected_disabled_theme =
            std::make_unique<UITheme>(info->m_selected_disabled_theme.value());
    }

    if (info->m_text) {
        m_text = std::make_unique<UIText>();
        m_text->m_resize_by_text = info->m_text->m_resize_by_text;
        m_text->m_color = info->m_text->m_color;
        m_text->m_align = info->m_text->m_align;
        m_text->SetFont(info->m_text->m_font);
        m_text->ChangeText(info->m_text->m_text);
    }

    if (info->m_text_input) {
        m_text_input = std::make_unique<UITextInput>();
        m_text_input->m_color = info->m_text_input->m_color;
        m_text_input->SetFont(info->m_text_input->m_font);
        m_text_input->SetFontPt(info->m_text_input->m_font_size);
        m_text_input->SetText(info->m_text_input->m_text);
        m_text_input->m_align = info->m_text_input->m_align;
    }
}

UIComponentManager::UIComponentManager() {
    m_text_input_event_listener =
        COMMON_CONTEXT.m_event_system->AddListener<SDL_TextInputEvent>(
            [this](EventListenerID id, const SDL_TextInputEvent& input) {
                this->handleTextInput(id, input);
            });
    m_key_event_listener =
        COMMON_CONTEXT.m_event_system->AddListener<SDL_KeyboardEvent>(
            [this](EventListenerID id, const SDL_KeyboardEvent& event) {
                this->handleKeyDown(id, event);
            });
}

UIComponentManager::~UIComponentManager() {
    COMMON_CONTEXT.m_event_system->RemoveListener<SDL_TextInputEvent>(
        m_text_input_event_listener);
    COMMON_CONTEXT.m_event_system->RemoveListener<SDL_KeyboardEvent>(
        m_key_event_listener);
}

void UIComponentManager::SetFocusedWidget(Entity entity) {
    TL_RETURN_IF_TRUE(m_focused_entity == entity);
    if (m_focused_entity != null_entity) {
        SDL_StopTextInput(CLIENT_CONTEXT.m_window->GetWindow());
    }
    m_focused_entity = entity;
    if (entity != null_entity) {
        SDL_StartTextInput(CLIENT_CONTEXT.m_window->GetWindow());
        m_cursor_visible = true;
        m_cursor_timer = 0;
    }
}

Entity UIComponentManager::GetFocusedWidget() const {
    return m_focused_entity;
}

bool UIComponentManager::IsFocusedWidget(Entity entity) const {
    return m_focused_entity == entity;
}

bool UIComponentManager::IsCursorVisible() const {
    return m_cursor_visible;
}

void UIComponentManager::Update(TimeType elapse_time) {
    PROFILE_SECTION();

    if (m_focused_entity != null_entity) {
        m_cursor_timer += static_cast<float>(elapse_time);
        if (m_cursor_timer >= CURSOR_BLINK_INTERVAL) {
            m_cursor_timer -= CURSOR_BLINK_INTERVAL;
            m_cursor_visible = !m_cursor_visible;
        }
    }

    auto level = CLIENT_CONTEXT.m_scene_manager->GetCurrentScene();
    TL_RETURN_IF_NULL(level);

    Entity ui_root_entity = level->GetUIRootEntity();
    Transform* transform =
        CLIENT_CONTEXT.m_transform_manager->Get(ui_root_entity);
    TL_RETURN_IF_TRUE(transform->m_size == Vec2::ZERO);
    updateSize(ui_root_entity);
    updateTransform(ui_root_entity);

    m_is_first_update = false;
}

void UIComponentManager::SubmitDrawCommand(Entity entity) {
    auto& renderer = CLIENT_CONTEXT.m_renderer;
    render(*renderer, entity);
}

void UIComponentManager::HandleEvent() {
    PROFILE_SECTION();

    auto level = CLIENT_CONTEXT.m_scene_manager->GetCurrentScene();
    if (!level) {
        return;
    }

    Entity ui_root_entity = level->GetUIRootEntity();
    auto relationship =
        CLIENT_CONTEXT.m_relationship_manager->Get(ui_root_entity);

    auto& mouse = CLIENT_CONTEXT.m_mouse;
    const Button& left_button = mouse->Get(MouseButtonType::Left);

#ifndef TL_ANDROID
    for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
        handleEvent(relationship->Get(i), -1, left_button, mouse->Position(),
                    mouse->Offset());
    }
#else
    auto& touches = CLIENT_CONTEXT.m_touches;
    for (size_t i = 0; i < touches->GetFingers().size(); i++) {
        auto& finger = touches->GetFingers()[i];
        if (finger.IsReleasing()) {
            continue;
        }

        for (uint32_t i = 0; i < relationship->GetChildrenCount(); i++) {
            Entity child = relationship->Get(i);
            UIWidget* child_ui = CLIENT_CONTEXT.m_ui_manager->Get(child);
            if (child_ui->m_focus_index && child_ui->m_focus_index != i) {
                continue;
            }
            handleEvent(child, i, finger, finger.Position(), finger.Offset());
        }
    }
#endif
}

void UIComponentManager::updateSize(Entity entity) {
    auto relationship = CLIENT_CONTEXT.m_relationship_manager->Get(entity);
    auto transform = CLIENT_CONTEXT.m_transform_manager->Get(entity);
    auto ui = Get(entity);

    TL_RETURN_IF_FALSE(transform && ui && relationship);

    if (ui->m_panel) {
        if (ui->m_old_transform.m_size == Vec2::ZERO) {
            ui->m_old_transform = *transform;
        }
        ui->m_panel->UpdateSize(ui->m_old_transform, *transform, *relationship,
                                *ui, m_is_first_update);
    }

    for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
        updateSize(relationship->Get(i));
    }
}

void UIComponentManager::updateTransform(Entity entity) {
    auto relationship = CLIENT_CONTEXT.m_relationship_manager->Get(entity);
    auto ui = Get(entity);
    auto transform = CLIENT_CONTEXT.m_transform_manager->Get(entity);

    TL_RETURN_IF_FALSE(transform && ui);

    TL_RETURN_IF_NULL(relationship);

    if (ui->m_panel) {
        if (ui->m_old_transform.m_size == Vec2::ZERO) {
            ui->m_old_transform = *transform;
        }
        ui->m_panel->UpdatePosition(ui->m_old_transform, *transform,
                                    *relationship, *ui, m_is_first_update);
    }

    for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
        updateTransform(relationship->Get(i));
    }

    ui->m_old_transform = *transform;
}

void UIComponentManager::handleEvent(Entity entity, size_t finger_index,
                                     const Button& button, const Vec2& position,
                                     const Vec2& offset) {
    auto transform = CLIENT_CONTEXT.m_transform_manager->Get(entity);
    auto ui = Get(entity);

    TL_RETURN_IF_FALSE(transform && ui);

    bool need_handle_event = true;
    auto relationship = CLIENT_CONTEXT.m_relationship_manager->Get(entity);
    if (!isFocusing(*ui, finger_index) && relationship) {
        for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
            Entity child = relationship->Get(i);
            auto child_transform =
                CLIENT_CONTEXT.m_transform_manager->Get(child);
            if (!child_transform) {
                continue;
            }

            Rect child_ui_rect;
            child_ui_rect.m_half_size = child_transform->m_size * 0.5;
            child_ui_rect.m_center =
                child_transform->m_position + child_ui_rect.m_half_size;
            if (IsPointInRect(position, child_ui_rect)) {
                need_handle_event = true;
                break;
            }
        }
    }

    if (ui->m_disabled) {
        need_handle_event = false;
    }

    if (!need_handle_event) {
        if (relationship) {
            for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
                handleEvent(relationship->Get(i), finger_index, button,
                            position, offset);
            }
        }
        return;
    }

    Rect ui_rect;
    ui_rect.m_half_size = transform->m_size * 0.5;
    ui_rect.m_center = transform->m_position + ui_rect.m_half_size;
    if (!isFocusing(*ui, finger_index) && !IsPointInRect(position, ui_rect)) {
        ui->m_state = UIState::Normal;
        return;
    }

    if (button.IsPressing() && isFocusing(*ui, finger_index) &&
        offset != Vec2::ZERO) {
        UIDragEvent event;
        event.m_entity = entity;
        CLIENT_CONTEXT.m_event_system->EnqueueEvent(event);
        return;
    }

    if (button.IsReleased()) {
        if (isFocusing(*ui, finger_index) &&
            button.GetLastUpTime() - button.GetLastDownTime() <= 0.1) {
            UIMouseClickedEvent event;
            event.m_entity = entity;
            CLIENT_CONTEXT.m_event_system->EnqueueEvent(event);
        }
        UIMouseUpEvent event{entity, button};
        CLIENT_CONTEXT.m_event_system->EnqueueEvent(event);
        ui->m_state = UIState::Normal;
        ui->m_focus_index.reset();
        return;
    }

    if (button.IsPressed()) {
        UIMouseDownEvent event{entity, button};
        CLIENT_CONTEXT.m_event_system->EnqueueEvent(event);
        ui->m_focus_index = finger_index;
        ui->m_state = UIState::Down;

        if (ui->m_text_input) {
            if (m_focused_entity != entity) {
                SetFocusedWidget(entity);
            }
        } else {
            if (m_focused_entity != null_entity) {
                SetFocusedWidget(null_entity);
            }
        }

        if (ui->m_can_be_selected) {
            ui->m_selected = !ui->m_selected;

            if (ui->m_selected) {
                UICheckToggledEvent event;
                event.m_checked = ui->m_selected;
                event.m_entity = entity;
                CLIENT_CONTEXT.m_event_system->EnqueueEvent(event);
            }
        }
        return;
    }

    if (!button.IsPress()) {
        UIMouseHoverEvent event;
        event.m_entity = entity;
        CLIENT_CONTEXT.m_event_system->EnqueueEvent(event);
        ui->m_state = UIState::Hover;
        return;
    }
}

void UIComponentManager::render(Renderer& renderer, Entity entity) {
    auto transform = CLIENT_CONTEXT.m_transform_manager->Get(entity);
    auto ui = Get(entity);

    TL_RETURN_IF_FALSE(transform && ui && IsEnable(entity) &&
                       ui->m_enable_draw);

    if (ui->m_use_clip) {
        SDL_Rect rect;
        rect.w = transform->m_size.w;
        rect.h = transform->m_size.h;
        rect.x = transform->m_position.x;
        rect.y = transform->m_position.y;
        SDL_SetRenderClipRect(renderer.GetRenderer(), &rect);
    }

    UITheme* theme = &ui->m_normal_theme;
    if (ui->m_state == UIState::Hover && ui->m_hover_theme) {
        theme = ui->m_hover_theme.get();
    } else if (ui->m_state == UIState::Down && ui->m_down_theme) {
        theme = ui->m_down_theme.get();
    } else if (ui->m_selected && ui->m_selected_theme) {
        theme = ui->m_selected_theme.get();
    } else if (ui->m_disabled && ui->m_disabled_theme) {
        if (ui->m_selected) {
            theme = ui->m_selected_disabled_theme.get();
        } else {
            theme = ui->m_disabled_theme.get();
        }
    }

    Rect rect;
    rect.m_half_size = transform->m_size * 0.5;
    rect.m_center = transform->m_position + rect.m_half_size;

    Region dst;
    dst.m_size = transform->m_size;
    dst.m_topleft = transform->m_position;

    const DrawOrder* draw_order =
        CLIENT_CONTEXT.m_draw_order_manager->Get(entity);
    double z_order = draw_order ? draw_order->GetGlobalOrder() : 0;
    float y = transform->m_position.y;

    if (theme->m_image) {
        Region src;
        src.m_size = theme->m_image->GetSize();
        theme->m_image->ChangeColorMask(theme->m_background_color);
        if (theme->m_image_9grid.m_scale > 0 &&
            theme->m_image_9grid.m_right > theme->m_image_9grid.m_left &&
            theme->m_image_9grid.m_top < theme->m_image_9grid.m_bottom) {
            renderer.DrawImage9Grid(
                *theme->m_image, src, dst, Color::White, theme->m_image_9grid,
                theme->m_image_9grid.m_scale, z_order, false, y);
        } else {
            renderer.DrawImage(*theme->m_image, src, dst, Color::White, 0, {},
                               Flip::None, z_order, false, y);
        }
        theme->m_image->ChangeColorMask(Color::White);
    } else {
        renderer.FillRect(rect, theme->m_background_color, z_order, false, y);
    }

    renderer.DrawRect(rect, theme->m_border_color, z_order, false, y);

    if (ui->m_text_input) {
        auto text_size = ui->m_text_input->GetTextImageSize();

        Region region;
        switch (ui->m_text_input->m_align) {
            case UITextAlign::Left:
                region.m_topleft.x = transform->m_position.x + ui->m_padding.x;
                break;
            case UITextAlign::Right:
                region.m_topleft.x = transform->m_position.x +
                                     transform->m_size.w - text_size.w -
                                     ui->m_padding.x;
                break;
            case UITextAlign::Center:
                region.m_topleft.x = transform->m_position.x +
                                     (transform->m_size.w - text_size.w) * 0.5;
                break;
        }
        region.m_size = text_size;
        region.m_topleft.y = rect.m_center.y - region.m_size.y * 0.5;

        Image& image = ui->m_text_input->GetTextImage();
        if (text_size.w > 0 && text_size.h > 0) {
            image.ChangeColorMask(theme->m_foreground_color);
            Region src;
            src.m_size = image.GetSize();
            renderer.DrawImage(image, src, region, Color::White, 0, Vec2::ZERO,
                               Flip::None, z_order, false, y);
            image.ChangeColorMask(Color::White);
        }

        if (IsFocusedWidget(entity) && IsCursorVisible()) {
            float cursor_x = ui->m_text_input->GetCursorX();
            float font_h =
                text_size.h > 0 ? static_cast<float>(text_size.h) : 16.0f;
            Rect cursor_rect;
            cursor_rect.m_half_size = Vec2{1.0f, font_h * 0.5f};
            cursor_rect.m_center =
                Vec2{region.m_topleft.x + cursor_x + 1, rect.m_center.y};
            renderer.FillRect(cursor_rect, theme->m_foreground_color, z_order,
                              false, y);
        }
    } else if (ui->m_text) {
        auto text_size = ui->m_text->GetTextImageSize();

        Region region;

        switch (ui->m_text->m_align) {
            case UITextAlign::Left:
                region.m_topleft.x = transform->m_position.x + ui->m_padding.x;
                break;
            case UITextAlign::Right:
                region.m_topleft.x = transform->m_position.x +
                                     transform->m_size.w - text_size.w -
                                     ui->m_padding.x;
                break;
            case UITextAlign::Center:
                region.m_topleft.x = transform->m_position.x +
                                     (transform->m_size.w - text_size.w) * 0.5;
                break;
        }

        Image& image = ui->m_text->GetTextImage();
        region.m_size = image.GetSize();
        region.m_topleft.y = rect.m_center.y - region.m_size.y * 0.5;
        image.ChangeColorMask(theme->m_foreground_color);
        Region src;
        src.m_size = image.GetSize();
        renderer.DrawImage(image, src, region, Color::White, 0, Vec2::ZERO,
                           Flip::None, z_order, false, y);
        image.ChangeColorMask(Color::White);
    }

    if (ui->m_use_clip) {
        SDL_SetRenderClipRect(renderer.GetRenderer(), NULL);
    }

    auto relationship = CLIENT_CONTEXT.m_relationship_manager->Get(entity);
    if (!relationship) {
        return;
    }

    for (size_t i = 0; i < relationship->GetChildrenCount(); i++) {
        render(renderer, relationship->Get(i));
    }
}

bool UIComponentManager::isFocusing(const UIWidget& ui,
                                    size_t finger_index) const {
    return ui.m_focus_index && ui.m_focus_index.value() == finger_index;
}

void UIComponentManager::handleTextInput(EventListenerID,
                                         const SDL_TextInputEvent& event) {
    TL_RETURN_IF_TRUE(m_focused_entity == null_entity);
    UIWidget* ui = Get(m_focused_entity);
    TL_RETURN_IF_FALSE(ui && ui->m_text_input);
    ui->m_text_input->HandleTextInput(event);
    m_cursor_visible = true;
    m_cursor_timer = 0;
}

void UIComponentManager::handleKeyDown(EventListenerID,
                                       const SDL_KeyboardEvent& event) {
    TL_RETURN_IF_FALSE(event.down && !event.repeat);
    TL_RETURN_IF_TRUE(m_focused_entity == null_entity);
    UIWidget* ui = Get(m_focused_entity);
    TL_RETURN_IF_FALSE(ui && ui->m_text_input);

    auto& text = *ui->m_text_input;
    switch (event.key) {
        case SDLK_BACKSPACE:
            text.DeleteBeforeCursor();
            break;
        case SDLK_DELETE:
            text.DeleteAfterCursor();
            break;
        case SDLK_LEFT:
            text.MoveCursorLeft();
            break;
        case SDLK_RIGHT:
            text.MoveCursorRight();
            break;
        case SDLK_HOME:
            text.MoveCursorHome();
            break;
        case SDLK_END:
            text.MoveCursorEnd();
            break;
        default:
            return;
    }
    m_cursor_visible = true;
    m_cursor_timer = 0;
}
