#pragma once
#include "entity.hpp"

#include "SDL3/SDL.h"
#include <optional>

class Window;
class Renderer;

class IInspector {
public:
    virtual ~IInspector() = default;
    virtual void Update() = 0;
};

class TrivialInspector : public IInspector {
public:
    ~TrivialInspector() override {}

    void Update() override {}
};

#ifdef TL_ENABLE_EDITOR

class Inspector : public IInspector {
public:
    Inspector(Window& window, Renderer& renderer);

    void Update() override;

private:
    Window& m_window;
    Renderer& m_renderer;
    bool m_hierarchy_window_open = true;
    bool m_detail_window_open = true;

    std::optional<Entity> m_selected_entity;

    void showEntityHierarchy(Entity node);
    void showEntityDetail(Entity entity);
};

#endif