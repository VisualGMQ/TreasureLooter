#pragma once
#include "entity.hpp"

#include <SDL3/SDL_events.h>
#include <optional>

class Window;
class Renderer;

class Inspector {
public:
    Inspector(Window& window, Renderer& renderer);
    ~Inspector();

    void BeginFrame();
    void EndFrame();
    void Update();

    void HandleEvents(const SDL_Event& event);

private:
    Window& m_window;
    Renderer& m_renderer;
    bool m_hierarchy_window_open = true;
    bool m_detail_window_open = true;

    std::optional<Entity> m_selected_entity;

    void showEntityHierarchy(Entity node);
    void showEntityDetail(Entity entity);
};