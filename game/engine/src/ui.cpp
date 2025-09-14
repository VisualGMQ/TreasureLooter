#include "engine/ui.hpp"

#include "engine/context.hpp"
#include "engine/relationship.hpp"

UIWidget::UIWidget(UIType type) : m_type{type} {}

void UIWidget::Render(const Transform& transform, Renderer& renderer) {
    if (m_use_clip) {
        SDL_Rect rect;
        rect.w = transform.m_size.w;
        rect.h = transform.m_size.h;
        rect.x = transform.m_position.x - rect.w * 0.5;
        rect.y = transform.m_position.y - rect.h * 0.5;
        SDL_SetRenderClipRect(renderer.GetRenderer(), &rect);
    }

    render(transform, renderer);

    if (m_use_clip) {
        SDL_SetRenderClipRect(renderer.GetRenderer(), NULL);
    }
}

UIType UIWidget::GetType() const {
    return m_type;
}

UIText::UIText() : UIWidget(UIType::Text) {}

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

const std::string& UIText::GetText() const {
    return m_text;
}

void UIText::HandleEvent() {}

void UIText::UpdateTransform(Transform& transform,
                             const Relationship* relationship) {}

void UIText::UpdateSize(Vec2& size) {
    if (m_resize_by_text) {
        size = m_text_image.GetSize();
    }
}

void UIText::render(const Transform& transform, Renderer& renderer) {
    if (!m_text_image.GetTexture()) {
        return;
    }

    Region dst;
    dst.m_size = transform.m_size;
    dst.m_topleft.y = transform.m_position.y +
                      (transform.m_size.h - m_text_image.GetSize().h) * 0.5;

    auto text_size = m_text_image.GetSize();

    switch (m_align) {
        case UITextAlign::Left:
            dst.m_topleft.x = transform.m_position.x;
            break;
        case UITextAlign::Right:
            dst.m_topleft.x =
                transform.m_position.x + transform.m_size.w - text_size.w;
            break;
        case UITextAlign::Center:
            dst.m_topleft.x = transform.m_position.x +
                              (transform.m_size.w - text_size.w) * 0.5;
            break;
    }

    m_text_image.ChangeColorMask(m_color);
    renderer.DrawImage(m_text_image,
                       {
                           {0, 0},
                           transform.m_size
    },
                       dst, 0, Vec2::ZERO, Flip::None, false);
    m_text_image.ChangeColorMask(Color::White);
}

void UIText::SetAlign(UITextAlign align) {
    m_align = align;
}

void UIText::regenerateText() {
    if (m_text.empty()) {
        return;
    }

    m_font->SetFontSize(m_pt);

    m_text_image =
        Image{*CURRENT_CONTEXT.m_renderer, m_font->GenerateText(m_text, m_color)};
}

UIButton::UIButton() : UIWidget(UIType::Button) {}

void UIButton::HandleEvent() {
    // TODO: not finish
}

void UIButton::UpdateTransform(Transform&, const Relationship*) {}

void UIButton::UpdateSize(Vec2& size) {
    if (m_image) {
        size = m_image->GetSize();
    }
}

void UIButton::render(const Transform& transform, Renderer& renderer) {
    Rect rect;
    rect.m_center = transform.m_position;
    rect.m_half_size = transform.m_size * 0.5;

    renderer.FillRect(rect, m_background_color, false);
    if (m_image) {
        Region src, dst;
        src.m_size = m_image->GetSize();
        src.m_topleft = Vec2::ZERO;
        dst.m_size = transform.m_size;
        dst.m_topleft = rect.m_center = rect.m_half_size;
        if (m_image_9grid.left != 0 && m_image_9grid.top != 0 &&
            m_image_9grid.right != 0 && m_image_9grid.bottom != 0) {
            renderer.DrawImage9Grid(*m_image.Get(), src, dst, m_image_9grid,
                                    false);
        } else {
            renderer.DrawImage(*m_image.Get(), src, dst, 0, Vec2::ZERO,
                               Flip::None, false);
        }
    }
    renderer.DrawRect(rect, m_border_color, false);
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
        updateSize(child);
    }

    for (auto child : relationship->m_children) {
        updateTransform(child);
    }
}

void UIComponentManager::Render() {
    auto level = CURRENT_CONTEXT.m_level_manager->GetCurrentLevel();
    if (!level) {
        return;
    }

    Entity ui_root_entity = level->GetUIRootEntity();
    auto& renderer = CURRENT_CONTEXT.m_renderer;
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(ui_root_entity);
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

    ui->UpdateSize(transform->m_size);

    if (!relationship) {
        return;
    }

    for (auto child : relationship->m_children) {
        updateSize(child);
    }
}

void UIComponentManager::updateTransform(Entity entity) {
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    auto transform = CURRENT_CONTEXT.m_transform_manager->Get(entity);
    auto ui = Get(entity);

    if (!transform || !ui) {
        return;
    }

    ui->UpdateTransform(*transform, relationship);

    if (!relationship) {
        return;
    }

    for (auto child : relationship->m_children) {
        updateSize(child);
    }
}

void UIComponentManager::render(Renderer& renderer, Entity entity) {
    auto relationship = CURRENT_CONTEXT.m_relationship_manager->Get(entity);
    auto transform = CURRENT_CONTEXT.m_transform_manager->Get(entity);
    auto ui = Get(entity);

    if (!transform || !ui) {
        return;
    }

    ui->Render(*transform, renderer);

    if (!relationship) {
        return;
    }

    for (auto child : relationship->m_children) {
        updateSize(child);
    }
}