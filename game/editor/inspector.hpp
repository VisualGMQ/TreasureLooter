#pragma once
#include "../engine/include/engine/entity.hpp"

#include "SDL3/SDL.h"
#include <optional>

class Window;
class Renderer;

class Inspector {
public:
    void Update();

private:
    bool m_hierarchy_window_open = true;
    bool m_detail_window_open = true;

    std::optional<Entity> m_selected_entity;

    void showEntityHierarchy(Entity node);
    void showEntityDetail(Entity entity);

    void showMenu();
};