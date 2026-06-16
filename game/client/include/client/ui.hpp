#pragma once
#include "client/font.hpp"
#include "client/image.hpp"
#include "client/renderer.hpp"
#include "common/entity.hpp"
#include "common/event.hpp"
#include "common/flag.hpp"
#include "common/image.hpp"
#include "common/manager.hpp"
#include "common/math.hpp"
#include "common/relationship.hpp"
#include "common/timer.hpp"
#include "common/utf8_string.hpp"
#include "input/button.hpp"
#include "schema/draw_order.hpp"
#include "schema/ui_config.hpp"

struct UIWidget;

class UITextInput {
public:
    UITextAlign m_align = UITextAlign::Left;
    Color m_color{0, 0, 0, 1};

    void SetFont(FontHandle font);
    void SetFontPt(uint32_t pt);

    const UTF8String& GetText() const;
    void SetText(const std::string& text);
    void SetText(const UTF8String& text);

    size_t GetCursorPos() const;
    void SetCursorPos(size_t pos);

    void MoveCursorLeft();
    void MoveCursorRight();
    void MoveCursorHome();
    void MoveCursorEnd();
    void DeleteBeforeCursor();
    void DeleteAfterCursor();

    Image& GetTextImage();
    Vec2 GetTextImageSize() const;
    void RefreshText();

    float GetCursorX();

    void HandleTextInput(const SDL_TextInputEvent& event);

private:
    FontHandle m_font;
    uint32_t m_pt = 16;
    UTF8String m_text;
    Image m_text_image;
    size_t m_cursor_pos = 0;

    mutable float m_cursor_x_cache = 0;
    mutable bool m_cursor_x_dirty = true;

    void regenerateText();
};

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
                            const UIWidget& ui, bool force) = 0;
    virtual void UpdatePosition(const Transform& old_transform,
                                const Transform& new_transform,
                                const Relationship& relationship,
                                const UIWidget& ui, bool force) = 0;
};

struct UIPanelComponent : public UILayer {
    void UpdateSize(const Transform& old_transform,
                    const Transform& new_transform,
                    const Relationship& relationship, const UIWidget& ui,
                    bool force) override;
    void UpdatePosition(const Transform& old_transform,
                        const Transform& new_transform,
                        const Relationship& relationship, const UIWidget& ui,
                        bool force) override;
};

struct UIBoxPanelComponent : public UIPanelComponent {
    UIBoxPanelType m_type = UIBoxPanelType::Vertical;
    float m_spacing = 0.0;

    void UpdateSize(const Transform& old_transform,
                    const Transform& new_transform,
                    const Relationship& relationship, const UIWidget& ui,
                    bool force) override;
    void UpdatePosition(const Transform& old_transform,
                        const Transform& new_transform,
                        const Relationship& relationship, const UIWidget& ui,
                        bool force) override;
};

struct UIWidget {
    friend class UIComponentManager;

    Flags<UIAnchor> m_anchor = UIAnchor::Center;
    bool m_enable_draw = true;
    bool m_use_clip = false;
    bool m_disabled = false;
    bool m_selected = false;
    bool m_can_be_selected = false;

    std::unique_ptr<UIText> m_text;
    std::unique_ptr<UITextInput> m_text_input;
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
    explicit UIWidget(UIWidgetDefinitionHandle);

private:
    Transform m_old_transform;

    /** For touch detect. When finger_touch/mouse down on widget and
        slide out, the widget shouldn't trigger press event
    **/
    std::optional<size_t> m_focus_index;  // -1 means mouse
};

struct UIMouseHoverEvent {
    Entity m_entity;
};

struct UIMouseDownEvent {
    Entity m_entity;
    const Button& m_button;
};

struct UIMouseUpEvent {
    Entity m_entity;
    const Button& m_button;
};

struct UIMouseClickedEvent {
    Entity m_entity;
};

struct UICheckToggledEvent {
    Entity m_entity;
    bool m_checked = false;
};

struct UIDragEvent {
    Entity m_entity;
};

class UIComponentManager : public ComponentManager<UIWidget> {
public:
    UIComponentManager();
    ~UIComponentManager() override;
    void Update(TimeType elapse_time);
    void SubmitDrawCommand(Entity);
    void HandleEvent();

    void SetFocusedWidget(Entity entity);
    Entity GetFocusedWidget() const;
    bool IsFocusedWidget(Entity entity) const;
    bool IsCursorVisible() const;

private:
    void updateSize(Entity);
    void updateTransform(Entity);
    /**
     * @param finger_index -1 means mouse
     */
    void handleEvent(Entity, size_t finger_index, const Button&,
                     const Vec2& position, const Vec2& offset);
    void render(Renderer&, Entity);
    bool isFocusing(const UIWidget&, size_t finger_index) const;

    void handleTextInput(EventListenerID, const SDL_TextInputEvent& e);
    void handleKeyDown(EventListenerID, const SDL_KeyboardEvent& e);

    static constexpr float CURSOR_BLINK_INTERVAL = 0.53f;

    bool m_is_first_update = true;
    Entity m_focused_entity{};

    // for UITextInput
    float m_cursor_timer = 0;
    bool m_cursor_visible = true;
    EventListenerID m_text_input_event_listener{};
    EventListenerID m_key_event_listener{};
};
