#include "common/detour/detour.hpp"

#include <queue>

void TilemapDetourManager::SetCurDetourData(TilemapDetourDataHandle handle) {
    m_data = handle;
}

TilemapDetourDataHandle TilemapDetourManager::GetCurDetourData() const {
    return m_data;
}

void TilemapDetourManager::Update() {}

bool BFSGridPathFinding::Build(const TilemapDetourData& detour_data,
                               Vec2I start_tile) {
    auto& storage = detour_data.m_data;
    m_storage.Clear();
    m_storage.Resize(storage.GetWidth(), storage.GetHeight());
    TL_RETURN_FALSE_IF_FALSE(m_storage.InRange(start_tile.x, start_tile.y));

    std::queue<Vec2I> queue;
    queue.push(start_tile);
    m_storage.Get(start_tile.x, start_tile.y).m_visited = true;
    bfs(detour_data, queue);

    return true;
}

const MatStorage<BFSGridPathFinding::PathFindingData>&
BFSGridPathFinding::GetPathFindingData() const {
    return m_storage;
}

void BFSGridPathFinding::bfs(const TilemapDetourData& detour_data,
                             std::queue<Vec2I>& queue) {
    static const std::array<Vec2I, 4> next_offsets{
        Vec2I{-1,  0},
        Vec2I{ 1,  0},
        Vec2I{ 0,  1},
        Vec2I{ 0, -1},
    };

    while (!queue.empty()) {
        Vec2I position = queue.front();
        queue.pop();
        auto& data = m_storage.Get(position.x, position.y);

        for (size_t idx = 0; idx < next_offsets.size(); idx++) {
            auto next_offset = next_offsets[idx];
            Vec2I next_position = position + next_offset;

            TL_CONTINUE_IF_FALSE(
                m_storage.InRange(next_position.x, next_position.y));

            DetourCostType tile_weight =
                detour_data.m_data.Get(next_position.x, next_position.y);

            TL_CONTINUE_IF_FALSE(tile_weight != detour_inf_cost);

            auto& next_data = m_storage.Get(next_position.x, next_position.y);
            if (!next_data.m_visited) {
                next_data.m_visited = true;
                next_data.m_parent = position;
                next_data.m_weight = data.m_weight + tile_weight;
                queue.push(next_position);
            }
        }
    }
}

bool DijkstraGridPathFinding::Build(const TilemapDetourData& detour_data,
                                    Vec2I start_tile) {
    auto& storage = detour_data.m_data;
    m_storage.Clear();
    m_storage.Resize(storage.GetWidth(), storage.GetHeight());
    TL_RETURN_FALSE_IF_FALSE(m_storage.InRange(start_tile.x, start_tile.y));

    m_heap.clear();
    m_heap.push_back({0, start_tile});
    std::push_heap(m_heap.begin(), m_heap.end(), std::greater<PQEntry>{});
    m_storage.Get(start_tile.x, start_tile.y).m_totle_cost = 0;
    bfs(detour_data);

    return true;
}

const MatStorage<DijkstraGridPathFinding::PathFindingData>&
DijkstraGridPathFinding::GetPathFindingData() const {
    return m_storage;
}

void DijkstraGridPathFinding::bfs(const TilemapDetourData& detour_data) {
    static const std::array<Vec2I, 8> next_offsets{
        Vec2I{-1,  0},
        Vec2I{ 1,  0},
        Vec2I{ 0,  1},
        Vec2I{ 0, -1},
        Vec2I{ 1,  1},
        Vec2I{ 1, -1},
        Vec2I{-1,  1},
        Vec2I{-1, -1},
    };

    while (!m_heap.empty()) {
        std::pop_heap(m_heap.begin(), m_heap.end(), std::greater<PQEntry>{});
        auto entry = m_heap.back();
        m_heap.pop_back();
        Vec2I position = entry.m_position;

        auto& data = m_storage.Get(position.x, position.y);
        TL_CONTINUE_IF_FALSE(entry.m_cost <= data.m_totle_cost);

        for (size_t i = 0; i < next_offsets.size(); i++) {
            auto next_offset = next_offsets[i];
            Vec2I next_position = position + next_offset;

            TL_CONTINUE_IF_FALSE(
                m_storage.InRange(next_position.x, next_position.y));

            DetourCostType tile_weight =
                detour_data.m_data.Get(next_position.x, next_position.y);

            TL_CONTINUE_IF_FALSE(tile_weight != detour_inf_cost);

            bool is_diagonal = (std::abs(next_offset.x * next_offset.y) == 1);
            if (is_diagonal) {
                DetourCostType adj1 = detour_data.m_data.Get(
                    position.x + next_offset.x, position.y);
                DetourCostType adj2 = detour_data.m_data.Get(
                    position.x, position.y + next_offset.y);
                TL_CONTINUE_IF_TRUE(adj1 == detour_inf_cost ||
                                    adj2 == detour_inf_cost);
            }

            DetourCostType edge_cost =
                is_diagonal ? tile_weight * 1.414f : tile_weight;
            DetourCostType new_cost = data.m_totle_cost + edge_cost;

            auto& next_data = m_storage.Get(next_position.x, next_position.y);
            if (new_cost < next_data.m_totle_cost) {
                next_data.m_totle_cost = new_cost;
                next_data.m_parent = position;
                m_heap.push_back({new_cost, next_position});
                std::push_heap(m_heap.begin(), m_heap.end(),
                               std::greater<PQEntry>{});
            }
        }
    }
}

bool AStarGridPathFinding::Build(const TilemapDetourData& detour_data,
                                 Vec2I start_tile, Vec2I end_tile,
                                 const HeuristicFn& h) {
    auto& storage = detour_data.m_data;
    m_storage.Clear();
    m_storage.Resize(storage.GetWidth(), storage.GetHeight());
    TL_RETURN_FALSE_IF_FALSE(m_storage.InRange(start_tile.x, start_tile.y));
    TL_RETURN_FALSE_IF_FALSE(m_storage.InRange(end_tile.x, end_tile.y));

    m_heap.clear();
    auto& start = m_storage.Get(start_tile.x, start_tile.y);
    start.m_totle_cost = 0;
    DetourCostType start_h = h(start_tile, end_tile);
    m_heap.push_back({start_h, 0, start_tile});
    std::push_heap(m_heap.begin(), m_heap.end(), std::greater<PQEntry>{});
    bfs(detour_data, h, end_tile);

    return true;
}

const MatStorage<AStarGridPathFinding::PathFindingData>&
AStarGridPathFinding::GetPathFindingData() const {
    return m_storage;
}

void AStarGridPathFinding::bfs(const TilemapDetourData& detour_data,
                                const HeuristicFn& h,
                                const Vec2I& end_position) {
    static const std::array<Vec2I, 8> next_offsets{
        Vec2I{-1,  0},  Vec2I{ 1,  0}, Vec2I{ 0,  1},  Vec2I{ 0, -1},
        Vec2I{ 1,  1},  Vec2I{ 1, -1}, Vec2I{-1,  1}, Vec2I{-1, -1},
    };

    while (!m_heap.empty()) {
        std::pop_heap(m_heap.begin(), m_heap.end(), std::greater<PQEntry>{});
        auto entry = m_heap.back();
        m_heap.pop_back();
        Vec2I position = entry.m_position;

        auto& data = m_storage.Get(position.x, position.y);
        TL_CONTINUE_IF_FALSE(entry.m_g <= data.m_totle_cost);

        if (position.x == end_position.x && position.y == end_position.y) {
            break;
        }

        for (size_t i = 0; i < next_offsets.size(); i++) {
            auto next_offset = next_offsets[i];
            Vec2I next_position = position + next_offset;

            TL_CONTINUE_IF_FALSE(
                m_storage.InRange(next_position.x, next_position.y));

            DetourCostType tile_weight =
                detour_data.m_data.Get(next_position.x, next_position.y);

            TL_CONTINUE_IF_FALSE(tile_weight != detour_inf_cost);

            bool is_diagonal = (std::abs(next_offset.x * next_offset.y) == 1);
            if (is_diagonal) {
                DetourCostType adj1 = detour_data.m_data.Get(
                    position.x + next_offset.x, position.y);
                DetourCostType adj2 = detour_data.m_data.Get(
                    position.x, position.y + next_offset.y);
                TL_CONTINUE_IF_TRUE(adj1 == detour_inf_cost ||
                                    adj2 == detour_inf_cost);
            }

            DetourCostType edge_cost =
                is_diagonal ? tile_weight * 1.414f : tile_weight;
            DetourCostType g = data.m_totle_cost + edge_cost;

            auto& next_data =
                m_storage.Get(next_position.x, next_position.y);
            if (g < next_data.m_totle_cost) {
                next_data.m_totle_cost = g;
                next_data.m_parent = position;
                DetourCostType f = g + h(next_position, end_position);
                m_heap.push_back({f, g, next_position});
                std::push_heap(m_heap.begin(), m_heap.end(),
                               std::greater<PQEntry>{});
            }
        }
    }
}