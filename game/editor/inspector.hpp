#pragma once
#include "engine/context.hpp"

#include <optional>

class Window;
class Renderer;

class Inspector {
public:
    explicit Inspector(CommonContext& context);
    void Update();

private:
    bool m_hierarchy_window_open = true;
    bool m_detail_window_open = true;
    CommonContext& m_context;

    std::optional<Entity> m_selected_entity;

    void showEntityHierarchy(Entity node);
    void showEntityDetail(Entity entity);
};