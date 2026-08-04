#include "client/detour.hpp"
#include "client/context.hpp"
#include "client/debug_drawer.hpp"
#include "client/input/mouse.hpp"
#include "client/renderer.hpp"
#include "common/debug_drawer.hpp"

void ClientTilemapDetourManager::RenderDebug() {
    renderDetourData();
    switch (m_debug_mode) {
        case DebugMode::None:
            break;
        case DebugMode::BFS:
            bfsInteractDebug();
            break;
        case DebugMode::Dijkstra:
            dijkstraInteractDebug();
            break;
        case DebugMode::AStar:
            astarInteractDebug();
            break;
    }

    renderSelectDetourPosition();
}

void ClientTilemapDetourManager::EnableDebugDraw(bool enable) {
    m_debug_draw_enabled = enable;
}

void ClientTilemapDetourManager::ToggleDebugDraw() {
    m_debug_draw_enabled = !m_debug_draw_enabled;
}

void ClientTilemapDetourManager::EnableBFSInteractDebug() {
    m_debug_mode = DebugMode::BFS;
}

void ClientTilemapDetourManager::EnableDijkstraInteractDebug() {
    m_debug_mode = DebugMode::Dijkstra;
}

void ClientTilemapDetourManager::EnableAStarInteractDebug() {
    m_debug_mode = DebugMode::AStar;
}

void ClientTilemapDetourManager::DisableInteractDebug() {
    m_debug_mode = DebugMode::None;
}

void ClientTilemapDetourManager::renderDetourData() {
    TL_RETURN_IF_FALSE(m_debug_draw_enabled);

    auto detour_data = GetCurDetourData();
    TL_RETURN_IF_FALSE(detour_data);

    auto& debug_draw = CLIENT_CONTEXT.m_debug_drawer;

    Color color = Color::Red;
    color.a = 0.5;

    for (uint32_t i = 0; i < detour_data->m_data.GetWidth(); i++) {
        for (uint32_t j = 0; j < detour_data->m_data.GetHeight(); j++) {
            auto tile = detour_data->m_data.Get(i, j);

            TL_CONTINUE_IF_FALSE(tile == detour_inf_cost)

            Rect rect;
            rect.m_center = Vec2(i, j) * Vec2{16, 16} + Vec2{8, 8};
            rect.m_half_size.w = 16 * 0.5;
            rect.m_half_size.h = 16 * 0.5;

            debug_draw->FillRect(rect, color, DebugDrawer::kOneFrame, true);
        }
    }
}

void ClientTilemapDetourManager::bfsInteractDebug() {
    TilemapDetourDataHandle detour_data = GetCurDetourData();
    TL_RETURN_IF_FALSE(detour_data);

    IDebugDrawer& debug_drawer = *CLIENT_CONTEXT.m_debug_drawer;
    if (debugDetourPositionSelect()) {
        m_debug_bfs.Build(*detour_data,
                          Vec2I(m_debug_begin.x, m_debug_begin.y));
    }

    auto& path_finding_data = m_debug_bfs.GetPathFindingData();
    TL_RETURN_IF_FALSE(path_finding_data.InRange(m_debug_end));
    auto& data = path_finding_data.Get(m_debug_end);
    TL_RETURN_IF_FALSE(data.m_visited);

    Vec2I cur = Vec2I(m_debug_end.x, m_debug_end.y);
    Rect rect;
    rect.m_half_size = {8, 8};
    Color color = Color::Purple;
    color.a = 0.5;
    while (cur.x != -1 && cur.y != -1) {
        rect.m_center = Vec2{16, 16} * Vec2(cur.x + 0.5f, cur.y + 0.5f);
        debug_drawer.FillRect(rect, color, IDebugDrawer::kOneFrame, true);
        cur = path_finding_data.Get(cur.x, cur.y).m_parent;
    }
}

void ClientTilemapDetourManager::dijkstraInteractDebug() {
    TilemapDetourDataHandle detour_data = GetCurDetourData();
    TL_RETURN_IF_FALSE(detour_data);

    IDebugDrawer& debug_drawer = *CLIENT_CONTEXT.m_debug_drawer;
    if (debugDetourPositionSelect()) {
        m_debug_dijkstra.Build(*detour_data,
                               Vec2I(m_debug_begin.x, m_debug_begin.y));
    }

    auto& path_finding_data = m_debug_dijkstra.GetPathFindingData();
    TL_RETURN_IF_FALSE(path_finding_data.InRange(m_debug_end));
    auto& data = path_finding_data.Get(m_debug_end);
    TL_RETURN_IF_FALSE(data.m_totle_cost != std::numeric_limits<DetourCostType>::max());

    Vec2I cur = Vec2I(m_debug_end.x, m_debug_end.y);
    Rect rect;
    rect.m_half_size = {8, 8};
    Color color = Color::Purple;
    color.a = 0.5;
    while (cur.x != -1 && cur.y != -1) {
        rect.m_center = Vec2{16, 16} * Vec2(cur.x + 0.5f, cur.y + 0.5f);
        debug_drawer.FillRect(rect, color, IDebugDrawer::kOneFrame, true);
        cur = path_finding_data.Get(cur.x, cur.y).m_parent;
    }
}

void ClientTilemapDetourManager::astarInteractDebug() {
    TilemapDetourDataHandle detour_data = GetCurDetourData();
    TL_RETURN_IF_FALSE(detour_data);

    IDebugDrawer& debug_drawer = *CLIENT_CONTEXT.m_debug_drawer;
    if (debugDetourPositionSelect()) {
        auto h = [](Vec2I current, Vec2I end) -> DetourCostType {
            return std::abs(current.x - end.x) + std::abs(current.y - end.y);
        };
        m_debug_astar.Build(*detour_data,
                            Vec2I(m_debug_begin.x, m_debug_begin.y),
                            Vec2I(m_debug_end.x, m_debug_end.y), h);
    }

    auto& path_finding_data = m_debug_astar.GetPathFindingData();
    TL_RETURN_IF_FALSE(path_finding_data.InRange(m_debug_end));
    auto& data = path_finding_data.Get(m_debug_end);
    TL_RETURN_IF_FALSE(data.m_totle_cost != std::numeric_limits<DetourCostType>::max());

    Vec2I cur = Vec2I(m_debug_end.x, m_debug_end.y);
    Rect rect;
    rect.m_half_size = {8, 8};
    Color color = Color::Purple;
    color.a = 0.5;
    while (cur.x != -1 && cur.y != -1) {
        rect.m_center = Vec2{16, 16} * Vec2(cur.x + 0.5f, cur.y + 0.5f);
        debug_drawer.FillRect(rect, color, IDebugDrawer::kOneFrame, true);
        cur = path_finding_data.Get(cur.x, cur.y).m_parent;
    }
}

bool ClientTilemapDetourManager::debugDetourPositionSelect() {
    auto mouse_position = CLIENT_CONTEXT.m_mouse->Position();
    if (CLIENT_CONTEXT.m_mouse->Get(MouseButtonType::Left).IsPressed()) {
        Vec2 world_position = CLIENT_CONTEXT.WindowCoordToWorld(mouse_position);
        m_debug_begin.w = std::floor(world_position.x / 16);
        m_debug_begin.h = std::floor(world_position.y / 16);
        return true;
    }
    if (CLIENT_CONTEXT.m_mouse->Get(MouseButtonType::Right).IsPressed()) {
        Vec2 world_position = CLIENT_CONTEXT.WindowCoordToWorld(mouse_position);
        m_debug_end.w = std::floor(world_position.x / 16);
        m_debug_end.h = std::floor(world_position.y / 16);
        return true;
    }
    return false;
}

void ClientTilemapDetourManager::renderSelectDetourPosition() {
    TL_RETURN_IF_FALSE(m_debug_mode != DebugMode::None);

    IDebugDrawer& debug_drawer = *CLIENT_CONTEXT.m_debug_drawer;

    debug_drawer.FillRect(
        Rect{
            Vec2{16, 16}
            * Vec2(m_debug_begin.x + 0.5f, m_debug_begin.y + 0.5f),
            { 8,  8}
    },
        Color::Red, DebugDrawer::kOneFrame, true);
    debug_drawer.FillRect(
        Rect{
            Vec2{16, 16}
            * Vec2(m_debug_end.x + 0.5f, m_debug_end.y + 0.5f),
            { 8,  8}
    },
        Color::Green, DebugDrawer::kOneFrame, true);
}
