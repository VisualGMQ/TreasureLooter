#pragma once

#include "common/tilemap.hpp"
#include "schema/detour.hpp"

#include <algorithm>
#include <queue>
#include <vector>

using DetourCostType = float;

class DetourInfCost {
public:
    constexpr bool operator==(DetourCostType value) const {
        return value == cost;
    }

    constexpr bool operator!=(DetourCostType value) const {
        return value != cost;
    }

    constexpr float GetUnderlyingValue() const { return cost; }

private:
    static constexpr float cost = -1;
};

constexpr bool operator==(DetourCostType value, DetourInfCost cost) {
    return cost == value;
}

constexpr bool operator!=(DetourCostType value, DetourInfCost cost) {
    return cost != value;
}

static constexpr DetourInfCost detour_inf_cost;

class TilemapDetourManager : public AssetManagerBase<TilemapDetourData> {
public:
    TilemapDetourDataHandle Load(const Path& filename,
                                 bool force = false) override;

    void SetCurDetourData(TilemapDetourDataHandle);
    [[nodiscard]] TilemapDetourDataHandle GetCurDetourData() const;

    void Update();

private:
    TilemapDetourDataHandle m_data;
};

class BFSGridPathFinding {
public:
    struct PathFindingData {
        Vec2I m_parent{-1, -1};
        DetourCostType m_weight = 0;
        bool m_visited = false;
    };

    bool Build(const TilemapDetourData&, Vec2I start_tile);
    [[nodiscard]] const MatStorage<PathFindingData>& GetPathFindingData() const;

private:
    MatStorage<PathFindingData> m_storage;

    void bfs(const TilemapDetourData&, std::queue<Vec2I>&);
};

class DijkstraGridPathFinding {
public:
    struct PathFindingData {
        Vec2I m_parent{-1, -1};
        DetourCostType m_totle_cost =
            std::numeric_limits<DetourCostType>::max();
    };

    struct PQEntry {
        DetourCostType m_cost;
        Vec2I m_position;

        bool operator>(const PQEntry& other) const {
            return m_cost > other.m_cost;
        }
    };

    using MinHeap = std::vector<PQEntry>;

    bool Build(const TilemapDetourData&, Vec2I start_tile);
    [[nodiscard]] const MatStorage<PathFindingData>& GetPathFindingData() const;

private:
    MatStorage<PathFindingData> m_storage;
    MinHeap m_heap;

    void bfs(const TilemapDetourData&);
};

class AStarGridPathFinding {
public:
    struct PathFindingData {
        Vec2I m_parent{-1, -1};
        DetourCostType m_totle_cost =
            std::numeric_limits<DetourCostType>::max();
    };

    struct PQEntry {
        DetourCostType m_f;
        DetourCostType m_g;
        Vec2I m_position;

        bool operator>(const PQEntry& other) const { return m_f > other.m_f; }
    };

    using MinHeap = std::vector<PQEntry>;
    using HeuristicFn = std::function<DetourCostType(Vec2I, Vec2I)>;

    bool Build(const TilemapDetourData&, Vec2I start_tile, Vec2I end_tile,
               const HeuristicFn& h);
    [[nodiscard]] const MatStorage<PathFindingData>& GetPathFindingData() const;

private:
    MatStorage<PathFindingData> m_storage;
    MinHeap m_heap;

    void bfs(const TilemapDetourData&, const HeuristicFn& h,
             const Vec2I& end_position);
};
