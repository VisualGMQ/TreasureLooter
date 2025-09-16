#include "engine/ui.hpp"

#include "engine/context.hpp"
#include "engine/relationship.hpp"

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

void UIText::SetAlign(UITextAlign align) {
    m_align = align;
}

UITextAlign UIText::GetAlign() const {
    return m_align;
}

void UIText::regenerateText() {
    if (m_text.empty()) {
        return;
    }

    m_font->SetFontSize(m_pt);

    m_text_image =
        Image{*CURRENT_CONTEXT.m_renderer,
              m_font->GenerateText(m_text, m_color)};
}

void UIBoxPanelComponent::UpdateSize(const Transform& transform,
                                     Relationship& relationship) {
    Vec2 start_position = transform.m_position + m_padding;
    Vec2 ceil_size = transform.m_size - (m_margin + m_padding) * 2.0;
    float totle_spacing = m_spacing * std::max(
                              (int)relationship.m_children.size() - 1, 0);
    if (m_type == UIBoxPanelType::Vertical) {
        ceil_size.h -= totle_spacing;
    } else {
        ceil_size.w -= totle_spacing;
    }

    for (int i = 0; i < relationship.m_children.size(); i++) {
        Entity entity = relationship.m_children[i];
        UIWidget* ui = CURRENT_CONTEXT.m_ui_manager->Get(entity);
        Transform* child_transform = CURRENT_CONTEXT.m_transform_manager->
            Get(entity);
        if (!child_transform || !ui) {
            continue;
        }

        child_transform->m_size = ceil_size;

        if (m_type == UIBoxPanelType::Vertical) {
            child_transform->m_position =
                start_position + (ceil_size + Vec2{0, m_spacing}) * i;
        } else {
            child_transform->m_position =
                start_position + (ceil_size + Vec2{m_spacing, 0}) * i;
        }
    }
}

UIWidget::UIWidget(const Vec2& position) {
    m_unscale_position = position;
}

void UIComponentManager::Update() {
    auto level = CURRENT_CONTEXT.m_level_manager->GetCurrentLevel();
    if (!level) {
        return;
    }

    Entity ui_root_entity = level->GetUIRootEntity();
    auto relationship =
        CURRENT_CONTEXT.m_relationship_manager->Get(ui_root_entity);

    for (auto child : relationship->m_children) {
        updateTransform(child, true);
    }

    for (auto child : relationship->m_children) {
        updateSize(child);
    }
}

void UIComponentManager::Render() {
    auto level = CURRENT_CONTEXT.m_level_manager->GetCurrentLevel();
    if (!level) {
        return;
    }

    Entity ui_root_entity = level->GetUIRootEntity();
    auto& renderer = CURRENT_CONTEXT.m_renderer;
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(
        ui_root_entity);
    for (auto child : relationship->m_children) {
        render(*renderer, child);
    }
}

void UIComponentManager::updateSize(Entity entity) {
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    auto transform = CURRENT_CONTEXT.m_transform_manager->Get(entity);
    auto ui = Get(entity);

    if (!transform || !ui) {
        return;
    }

    if (ui->m_panel) {
        ui->m_panel->UpdateSize(*transform, *relationship);
    }

    if (!relationship) {
        return;
    }

    for (auto child : relationship->m_children) {
        updateSize(child);
    }
}

void UIComponentManager::updateTransform(Entity entity,
                                         bool need_scale_position) {
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    auto ui = Get(entity);

    if (need_scale_position) {
        auto transform = CURRENT_CONTEXT.m_transform_manager->Get(entity);

        if (!transform || !ui) {
            return;
        }

        Vec2 logic_size(CURRENT_CONTEXT.GetGameConfig().m_logic_size.x,
                        CURRENT_CONTEXT.GetGameConfig().m_logic_size.y);
        Vec2 window_size = logic_size;
        if (&CURRENT_CONTEXT == &GAME_CONTEXT) {
            window_size = GAME_CONTEXT.m_window->GetWindowSize();
        }
        auto anchor = ui->m_anchor;
        if ((anchor & UIAnchor::Left && anchor & UIAnchor::Right) ||
            (anchor & UIAnchor::Top && anchor & UIAnchor::Bottom)) {
            LOGE("invalid ui anchor");
        }

        Vec2 position_offset_scale = window_size / logic_size;
        Vec2 anchor_offset;

        if (anchor & UIAnchor::Left) {
            anchor_offset.x = ui->m_unscale_position.x;
        } else if (anchor & UIAnchor::Right) {
            anchor_offset.x = logic_size.w - ui->m_unscale_position.x;
        }

        if (anchor & UIAnchor::Top) {
            anchor_offset.y = ui->m_unscale_position.y;
        } else if (anchor & UIAnchor::Bottom) {
            anchor_offset.y = logic_size.h - ui->m_unscale_position.y;
        }

        anchor_offset = anchor_offset * position_offset_scale;

        if (anchor & UIAnchor::Left) {
            transform->m_position.x = anchor_offset.x;
        } else if (anchor & UIAnchor::Right) {
            transform->m_position.x = window_size.w - anchor_offset.x;
        }

        if (anchor & UIAnchor::Top) {
            transform->m_position.y = anchor_offset.y;
        } else if (anchor & UIAnchor::Bottom) {
            transform->m_position.y = window_size.h - anchor_offset.y;
        }
    }

    if (!relationship) {
        return;
    }

    for (auto child : relationship->m_children) {
        updateTransform(child, ui && ui->m_panel != nullptr);
    }
}

void UIComponentManager::handleEvent(Entity) {
    // TODO: 
}

void UIComponentManager::render(Renderer& renderer, Entity entity) {
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    auto transform = CURRENT_CONTEXT.m_transform_manager->Get(entity);
    auto ui = Get(entity);

    if (!transform || !ui) {
        return;
    }
    if (ui->m_use_clip) {
        SDL_Rect rect;
        rect.w = transform->m_size.w;
        rect.h = transform->m_size.h;
        rect.x = transform->m_position.x - rect.w * 0.5;
        rect.y = transform->m_position.y - rect.h * 0.5;
        SDL_SetRenderClipRect(renderer.GetRenderer(), &rect);
    }

    UITheme* theme = &ui->m_normal_theme;
    if (ui->m_state == UIState::Hover && ui->m_hover_theme) {
        theme = ui->m_hover_theme.get();
    }
    if (ui->m_state == UIState::Down && ui->m_down_theme) {
        theme = ui->m_down_theme.get();
    }

    Rect rect;
    rect.m_half_size = transform->m_size * 0.5;
    rect.m_center = transform->m_position + rect.m_half_size;

    Region dst;
    dst.m_size = rect.m_half_size * 2.0;
    dst.m_topleft = rect.m_center - rect.m_half_size;

    if (ui->m_panel) {
        if (theme->m_image) {
            if (theme->m_image_9grid) {
                renderer.DrawImage9Grid(*theme->m_image, {}, dst,
                                        theme->m_image_9grid, false);
            } else {
                renderer.DrawImage(*theme->m_image, {}, dst, 0, {}, Flip::None,
                                   false);
            }
        } else {
            renderer.FillRect(rect, theme->m_background_color, false);
        }

        renderer.DrawRect(rect, theme->m_border_color, false);
    }

    if (ui->m_text) {
        auto text_size = ui->m_text->GetTextImageSize();

        Region region = dst;

        switch (ui->m_text->GetAlign()) {
            case UITextAlign::Left:
                region.m_topleft.x = transform->m_position.x;
                break;
            case UITextAlign::Right:
                region.m_topleft.x =
                    transform->m_position.x + transform->m_size.w - text_size.w;
                break;
            case UITextAlign::Center:
                region.m_topleft.x = transform->m_position.x +
                                     (transform->m_size.w - text_size.w) * 0.5;
                break;
        }

        Image& image = ui->m_text->GetTextImage();
        image.ChangeColorMask(theme->m_foreground_color);
        renderer.DrawImage(image,
                           {
                               {0, 0},
                               transform->m_size
                           },
                           region, 0, Vec2::ZERO, Flip::None, false);
        image.ChangeColorMask(Color::White);
    }

    if (ui->m_use_clip) {
        SDL_SetRenderClipRect(renderer.GetRenderer(), NULL);
    }

    if (!relationship) {
        return;
    }

    for (auto child : relationship->m_children) {
        render(renderer, child);
    }
}
