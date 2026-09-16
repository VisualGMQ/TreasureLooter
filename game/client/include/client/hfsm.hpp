#pragma once

#include "common/entity.hpp"

#include <memory>
#include <unordered_map>
#include <unordered_set>

/**
 * @brief Runtime visualizer for HFSM components.
 *
 * Draws the hierarchy of a HFSMComponent (nodes and parent-child connections)
 * with ImNodeFlow. Each toggled entity owns one standalone window named after
 * the HFSM asset.
 */
class ClientHFSMDebugger {
public:
    ClientHFSMDebugger();
    ~ClientHFSMDebugger();

    ClientHFSMDebugger(const ClientHFSMDebugger&) = delete;
    ClientHFSMDebugger& operator=(const ClientHFSMDebugger&) = delete;
    ClientHFSMDebugger(ClientHFSMDebugger&&) = delete;
    ClientHFSMDebugger& operator=(ClientHFSMDebugger&&) = delete;

    /// Show or hide the HFSM window of the given entity.
    void ToggleVisible(LogicEntity entity);

    /// Draw every visible HFSM window. Must be called inside an ImGui frame.
    void Render();

private:
    struct Graph;

    [[nodiscard]] Graph& GetGraph(LogicEntity entity);
    void RenderGraph(Graph& graph, class HFSMComponent& component);
    [[nodiscard]] bool RenderWindow(LogicEntity entity, class HFSMComponent& component);

    std::unordered_map<LogicEntity, std::unique_ptr<Graph>> m_graphs;
    std::unordered_set<LogicEntity> m_visible;
};
