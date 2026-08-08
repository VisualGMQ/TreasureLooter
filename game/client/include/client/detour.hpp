#pragma once

#include "common/detour/detour.hpp"

class ClientTilemapDetourManager : public TilemapDetourManager {
public:
    void RenderDebug();
    void EnableDebugDraw(bool);
    void ToggleDebugDraw();
    void EnableBFSInteractDebug();
    void EnableDijkstraInteractDebug();
    void EnableAStarInteractDebug();
    void DisableInteractDebug();

private:
    bool m_debug_draw_enabled = false;

    enum class DebugMode {
        None,
        BFS,
        Dijkstra,
        AStar,
    } m_debug_mode = DebugMode::None;

    BFSGridPathFinding m_debug_bfs;
    DijkstraGridPathFinding m_debug_dijkstra;
    AStarGridPathFinding m_debug_astar;
    Vec2UI m_debug_begin;
    Vec2UI m_debug_end;

    void renderDetourData();

    void bfsInteractDebug();
    void dijkstraInteractDebug();
    void astarInteractDebug();

    bool debugDetourPositionSelect();
    void renderSelectDetourPosition();
};
