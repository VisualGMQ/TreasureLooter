#pragma once
#include "engine/flag.hpp"
#include "engine/image.hpp"
#include "engine/manager.hpp"
#include "engine/math.hpp"
#include "engine/renderer.hpp"
#include "schema/relationship.hpp"
#include "engine/text.hpp"
#include "schema/ui_config.hpp"

enum class UITextAlign {
    Left,
    Right,
    Center,
};

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

    void SetAlign(UITextAlign align);
    UITextAlign GetAlign() const;

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
    
    virtual void UpdateSize(const Transform& transform,
                            Relationship& relatioship) = 0;

    Vec2 m_margin;
    Vec2 m_padding;
};

/**
* @brief don't layout child, only a container
*/
struct UIPanelComponent : public UILayer {
    void UpdateSize(const Transform& transform, Relationship& relatioship) override {}
};

enum class UIBoxPanelType {
    Vertical,
    Horizontal,
};

class UIBoxPanelComponent : public UIPanelComponent {
public:
    UIBoxPanelType m_type = UIBoxPanelType::Vertical;
    float m_spacing = 0.0;

    void UpdateSize(const Transform& transform, Relationship& relatioship) override;
};

struct UITheme {
    Color m_background_color = {1, 1, 1, 1};
    Color m_border_color = {0, 0, 0, 1};
    Color m_foreground_color = {1, 1, 1, 1};
    ImageHandle m_image;
    Image9Grid m_image_9grid;
};

enum class UIState {
    Normal,
    Hover,
    Down,
};

struct UIWidget {
    Flags<UIAnchor> m_anchor = UIAnchor::Center;
    bool m_use_clip = false;

    std::unique_ptr<UIText> m_text;
    std::unique_ptr<UIPanelComponent> m_panel;

    UITheme m_normal_theme;
    std::unique_ptr<UITheme> m_hover_theme;
    std::unique_ptr<UITheme> m_down_theme;

    UIState m_state = UIState::Normal;
    Vec2 m_unscale_position;

    explicit UIWidget(const Vec2& position);
};

class UIComponentManager : public ComponentManager<UIWidget> {
public:
    void Update();
    void Render();

private:
    void updateSize(Entity);
    void updateTransform(Entity, bool need_scale_position);
    void handleEvent(Entity);
    void render(Renderer&, Entity);
};
