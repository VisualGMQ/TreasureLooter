#pragma once

#include "common/entity.hpp"

#include <memory>
#include <unordered_map>
#include <unordered_set>

class ClientBTDebugger {
public:
    ClientBTDebugger();
    ~ClientBTDebugger();

    ClientBTDebugger(const ClientBTDebugger&) = delete;
    ClientBTDebugger& operator=(const ClientBTDebugger&) = delete;
    ClientBTDebugger(ClientBTDebugger&&) = delete;
    ClientBTDebugger& operator=(ClientBTDebugger&&) = delete;

    void ToggleVisible(LogicEntity entity);

    void Render();

private:
    struct Graph;

    [[nodiscard]] Graph& GetGraph(LogicEntity entity);
    void RenderGraph(Graph& graph, class BehaviorTreeComponent& component);
    [[nodiscard]] bool RenderWindow(LogicEntity entity,
                                    class BehaviorTreeComponent& component);

    std::unordered_map<LogicEntity, std::unique_ptr<Graph>> m_graphs;
    std::unordered_set<LogicEntity> m_visible;
};
