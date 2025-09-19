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

    m_text_image = Image{*CURRENT_CONTEXT.m_renderer,
                         m_font->GenerateText(m_text, m_color)};
}

void UIPanelComponent::UpdateSize(const Transform& old_transform,
                                  const Transform& new_transform,
                                  const Relationship& relationship,
                                  const UIWidget& ui) {
    if (old_transform.m_size == new_transform.m_size) {
        return;
    }

    for (auto child : relationship.m_children) {
        Transform* transform = CURRENT_CONTEXT.m_transform_manager->Get(child);
        UIWidget* child_ui = CURRENT_CONTEXT.m_ui_manager->Get(child);

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
            transform->m_size.w = new_transform.m_size.w - top - bottom;
        }
    }
}

void UIPanelComponent::UpdatePosition(const Transform& old_transform,
                                      const Transform& new_transform,
                                      const Relationship& relationship,
                                      const UIWidget& ui) {
    if (old_transform.m_size == new_transform.m_size) {
        return;
    }
    for (auto child : relationship.m_children) {
        Transform* child_transform =
            CURRENT_CONTEXT.m_transform_manager->Get(child);
        UIWidget* child_ui = CURRENT_CONTEXT.m_ui_manager->Get(child);

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

        if (child_ui->m_anchor & UIAnchor::Left) {
            float offset =
                child_transform->m_position.x - old_transform.m_position.x;
            child_transform->m_position.x = new_transform.m_position.x + offset;
        }
        if (child_ui->m_anchor & UIAnchor::Right) {
            float offset =
                child_transform->m_position.x -
                (old_transform.m_position.x + old_transform.m_size.w);
            child_transform->m_position.x =
                new_transform.m_position.x + new_transform.m_size.w + offset;
        }

        if (child_ui->m_anchor & UIAnchor::Top) {
            float offset =
                child_transform->m_position.y - old_transform.m_position.y;
            child_transform->m_position.y = new_transform.m_position.y + offset;
        }

        if (child_ui->m_anchor & UIAnchor::Bottom) {
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
                                     const UIWidget& ui) {
    if (old_transform == new_transform) {
        return;
    }

    Vec2 ceil_size = new_transform.m_size - ui.m_padding * 2.0;
    float totle_spacing =
        m_spacing * std::max((int)relationship.m_children.size() - 1, 0);
    if (m_type == UIBoxPanelType::Vertical) {
        ceil_size.h -= totle_spacing;
    } else {
        ceil_size.w -= totle_spacing;
    }
    ceil_size -= ui.m_margin * 2.0;

    for (int i = 0; i < relationship.m_children.size(); i++) {
        Entity entity = relationship.m_children[i];
        UIWidget* ui = CURRENT_CONTEXT.m_ui_manager->Get(entity);
        Transform* child_transform =
            CURRENT_CONTEXT.m_transform_manager->Get(entity);
        if (!child_transform || !ui) {
            continue;
        }

        child_transform->m_size = ceil_size;
    }
}

void UIBoxPanelComponent::UpdatePosition(const Transform& old_transform,
                                         const Transform& new_transform,
                                         const Relationship& relationship,
                                         const UIWidget& ui) {
    if (old_transform == new_transform) {
        return;
    }

    Vec2 start_position = new_transform.m_position + ui.m_padding;

    for (int i = 0; i < relationship.m_children.size(); i++) {
        Entity entity = relationship.m_children[i];
        UIWidget* ui = CURRENT_CONTEXT.m_ui_manager->Get(entity);
        Transform* child_transform =
            CURRENT_CONTEXT.m_transform_manager->Get(entity);
        if (!child_transform || !ui) {
            continue;
        }

        if (m_type == UIBoxPanelType::Vertical) {
            child_transform->m_position =
                start_position +
                (child_transform->m_size + Vec2{0, m_spacing}) * i;
        } else {
            child_transform->m_position =
                start_position +
                (child_transform->m_size + Vec2{m_spacing, 0}) * i;
        }
        child_transform->m_position += ui->m_margin * 2.0;
    }
}

UIWidget::UIWidget() {
    m_old_transform.m_size = Vec2::ZERO;
}

void UIComponentManager::Update() {
    auto level = CURRENT_CONTEXT.m_level_manager->GetCurrentLevel();
    if (!level) {
        return;
    }

    Entity ui_root_entity = level->GetUIRootEntity();
    Transform* transform = CURRENT_CONTEXT.m_transform_manager->Get(
        ui_root_entity);
    if (transform->m_size == Vec2::ZERO) {
        return;
    }
    updateSize(ui_root_entity);
    updateTransform(ui_root_entity);
}

void UIComponentManager::Render() {
    auto level = CURRENT_CONTEXT.m_level_manager->GetCurrentLevel();
    if (!level) {
        return;
    }

    Entity ui_root_entity = level->GetUIRootEntity();
    auto& renderer = CURRENT_CONTEXT.m_renderer;
    auto relationship =
        CURRENT_CONTEXT.m_relationship_manager->Get(ui_root_entity);
    for (auto child : relationship->m_children) {
        render(*renderer, child);
    }
}

void UIComponentManager::updateSize(Entity entity) {
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    auto transform = CURRENT_CONTEXT.m_transform_manager->Get(entity);
    auto ui = Get(entity);

    if (!transform || !ui || !relationship) {
        return;
    }

    if (ui->m_panel) {
        if (ui->m_old_transform.m_size == Vec2::ZERO) {
            ui->m_old_transform = *transform;
        }
        ui->m_panel->UpdateSize(ui->m_old_transform, *transform, *relationship, *ui);
    }

    for (auto child : relationship->m_children) {
        updateSize(child);
    }
}

void UIComponentManager::updateTransform(Entity entity) {
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    auto ui = Get(entity);
    auto transform = CURRENT_CONTEXT.m_transform_manager->Get(entity);

    if (!transform || !ui) {
        return;
    }

    if (!relationship) {
        return;
    }

    if (ui->m_panel) {
        if (ui->m_old_transform.m_size == Vec2::ZERO) {
            ui->m_old_transform = *transform;
        }
        ui->m_panel->UpdatePosition(ui->m_old_transform, *transform,
                                    *relationship, *ui);
    }

    for (auto child : relationship->m_children) {
        updateTransform(child);
    }

    ui->m_old_transform = *transform;
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
    dst.m_size = transform->m_size;
    dst.m_topleft = transform->m_position;

    if (ui->m_panel) {
        if (theme->m_image) {
            Region src;
            src.m_size = theme->m_image->GetSize();
            if (theme->m_image_9grid) {
                renderer.DrawImage9Grid(*theme->m_image, src, dst,
                                        theme->m_image_9grid, false);
            } else {
                renderer.DrawImage(*theme->m_image, src, dst, 0, {}, Flip::None,
                                   false);
            }
        } else {
            renderer.FillRect(rect, theme->m_background_color, false);
        }

        renderer.DrawRect(rect, theme->m_border_color, false);
    }

    if (ui->m_text) {
        auto text_size = ui->m_text->GetTextImageSize();

        Region region;

        switch (ui->m_text->GetAlign()) {
            case UITextAlign::Left:
                region.m_topleft.x = transform->m_position.x + ui->m_padding.x;
                break;
            case UITextAlign::Right:
                region.m_topleft.x =
                    transform->m_position.x + transform->m_size.w - text_size.w - ui->m_padding.x;
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
        renderer.DrawImage(image, src, region, 0, Vec2::ZERO, Flip::None,
                           false);
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
