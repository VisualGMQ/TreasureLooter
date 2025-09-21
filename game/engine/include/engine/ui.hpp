#pragma once
#include "engine/flag.hpp"
#include "engine/image.hpp"
#include "engine/manager.hpp"
#include "engine/math.hpp"
#include "engine/renderer.hpp"
#include "engine/text.hpp"
#include "input/button.hpp"
#include "schema/relationship.hpp"
#include "schema/ui_config.hpp"
#include "timer.hpp"

struct UIWidget;

class UIText {
public:
    UITextAlign m_align = UITextAlign::Center;
    bool m_resize_by_text = false;
    Color m_color{0, 0, 0, 1};

    void SetFont(FontHandle);
    void ChangeText(const std::string& text);
    void ChangeTextPt(uint32_t pt);
    Vec2 GetTextImageSize() const;
    [[nodiscard]] const std::string& GetText() const;
    Image& GetTextImage();

private:
    FontHandle m_font;
    uint32_t m_pt = 16;
    std::string m_text;
    Image m_text_image;

    void regenerateText();
};

class UILayer {
public:
    virtual ~UILayer() = default;

    virtual void UpdateSize(const Transform& old_transform,
                            const Transform& new_transform,
                            const Relationship& relationship,
                            const UIWidget& ui,
                            bool force) = 0;
    virtual void UpdatePosition(const Transform& old_transform,
                                const Transform& new_transform,
                                const Relationship& relationship,
                                const UIWidget& ui,
                                bool force) = 0;
};

struct UIPanelComponent : public UILayer {
    void UpdateSize(const Transform& old_transform,
                    const Transform& new_transform,
                    const Relationship& relationship,
                    const UIWidget& ui,
                    bool force) override;
    void UpdatePosition(const Transform& old_transform,
                        const Transform& new_transform,
                        const Relationship& relationship,
                        const UIWidget& ui,
                        bool force) override;
};

struct UIBoxPanelComponent : public UIPanelComponent {
    UIBoxPanelType m_type = UIBoxPanelType::Vertical;
    float m_spacing = 0.0;

    void UpdateSize(const Transform& old_transform,
                    const Transform& new_transform,
                    const Relationship& relationship,
                    const UIWidget& ui,
                    bool force) override;
    void UpdatePosition(const Transform& old_transform,
                        const Transform& new_transform,
                        const Relationship& relationship,
                        const UIWidget& ui,
                        bool force) override;
};

struct UIWidget {
    friend class UIComponentManager;
    
    Flags<UIAnchor> m_anchor = UIAnchor::Center;
    bool m_use_clip = false;
    bool m_disabled = false;
    bool m_selected = false;
    bool m_can_be_selected = false;

    std::unique_ptr<UIText> m_text;
    std::unique_ptr<UIPanelComponent> m_panel;

    UITheme m_normal_theme;
    std::unique_ptr<UITheme> m_hover_theme;
    std::unique_ptr<UITheme> m_down_theme;
    std::unique_ptr<UITheme> m_disabled_theme;
    std::unique_ptr<UITheme> m_selected_theme;
    std::unique_ptr<UITheme> m_selected_disabled_theme;

    UIState m_state = UIState::Normal;
    Vec2 m_margin;
    Vec2 m_padding;

    UIWidget();
    explicit UIWidget(UIWidgetInfoHandle);

private:
    Transform m_old_transform;

    std::optional<size_t> m_focus_index; // -1 means mouse
};

struct UIMouseHoverEvent {
    Entity m_entity;
};

struct UIMouseDownEvent{
    Entity m_entity;
    const Button& m_button;
};

struct UIMouseUpEvent{
    Entity m_entity;
    const Button& m_button;
};

struct UIMouseClickedEvent{
    Entity m_entity;
};

struct UICheckToggledEvent{
    Entity m_entity;
    bool m_checked = false;
};

struct UIDragEvent{
    Entity m_entity;
};

class UIComponentManager : public ComponentManager<UIWidget> {
public:
    void Update();
    void Render();
    void HandleEvent();

private:
    void updateSize(Entity);
    void updateTransform(Entity);
    /**
     * @param finger_index -1 means mouse
     */
    void handleEvent(Entity, size_t finger_index, const Button&, const Vec2& position, const Vec2& offset);
    void render(Renderer&, Entity);
    bool isFocus(const UIWidget&, size_t finger_index) const;

    bool m_is_first_update = true;
};
