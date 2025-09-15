#pragma once
#include "engine/flag.hpp"
#include "engine/image.hpp"
#include "engine/manager.hpp"
#include "engine/math.hpp"
#include "engine/renderer.hpp"
#include "schema/relationship.hpp"
#include "engine/text.hpp"
#include "schema/ui_config.hpp"

class UIWidget {
public:
    explicit UIWidget(UIType);
    virtual ~UIWidget() = default;
    virtual void UpdateTransform(Transform&, const Relationship*) = 0;
    virtual void UpdateSize(Vec2& size) = 0;
    virtual void HandleEvent() = 0;
    void Render(const Transform&, Renderer&);
    UIType GetType() const;

    bool m_use_clip = false;
    Flags<UIAnchor> m_anchor = UIAnchor::Center;

protected:
    virtual void render(const Transform&, Renderer&) = 0;

private:
    UIType m_type;
};

enum class UITextAlign {
    Left,
    Right,
    Center,
};

class UIPanel: public UIWidget {
    // TODO: not finish
};

class UIGridPanel: public UIWidget {
    // TODO: not finish
};

class UIText : public UIWidget {
public:
    UITextAlign m_align = UITextAlign::Center;
    bool m_resize_by_text = false;
    Color m_color{0, 0, 0, 1};

    UIText();
    void SetFont(FontHandle);
    void ChangeText(const std::string& text);
    void ChangeTextPt(uint32_t pt);
    [[nodiscard]] const std::string& GetText() const;

    void HandleEvent() override;
    void UpdateTransform(Transform&, const Relationship*) override;
    void UpdateSize(Vec2& size) override;
    void SetAlign(UITextAlign align);

protected:
    void render(const Transform&, Renderer&) override;

private:
    FontHandle m_font;
    uint32_t m_pt = 16;
    std::string m_text;
    Image m_text_image;

    void regenerateText();
};

class UIButton : public UIWidget {
public:
    UIButton();
    Color m_background_color = {1, 1, 1, 1};
    Color m_border_color = {0, 0, 0, 1};
    ImageHandle m_image;
    Image9Grid m_image_9grid;
    UIText m_text;
    Vec2 m_margin{16, 16};
    bool m_fit_text{false};

    void HandleEvent() override;
    void UpdateTransform(Transform&, const Relationship*) override;
    void UpdateSize(Vec2& size) override;

protected:
    void render(const Transform&, Renderer&) override;
};

class UIComponentManager: public ComponentManager<UIWidget> {
public:
    void Update();
    void Render();

private:
    void updateSize(Entity);
    void updateTransform(Entity);
    void render(Renderer&, Entity);
};