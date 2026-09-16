#include "client/hfsm.hpp"

#include "common/context.hpp"
#include "common/hfsm.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"

#include "ImNodeFlow.h"
#include "imgui.h"

#include <algorithm>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

constexpr float kNodeSpacingX = 220.f;
constexpr float kNodeSpacingY = 150.f;
constexpr ImU32 kCurrentHeaderColor = IM_COL32(224, 184, 58, 255);
constexpr ImU32 kIdleHeaderColor = IM_COL32(71, 142, 173, 255);

class HFSMNodeView : public ImFlow::BaseNode {
public:
    HFSMNodeView(HFSMNodeID id, std::string name) : m_hfsm_id(id) {
        setTitle(std::move(name));
        addIN<int>("", 0, ImFlow::ConnectionFilter::None());
        (void)addOUT<int>("");
    }

    [[nodiscard]] HFSMNodeID GetHFSMNodeID() const { return m_hfsm_id; }

private:
    HFSMNodeID m_hfsm_id;
};

std::vector<HFSMNodeID> CollectNodeIDs(const HFSMComponent& component) {
    std::vector<HFSMNodeID> ids;
    ids.reserve(component.GetNodes().size());
    for (const auto& [id, node] : component.GetNodes()) {
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

}  // namespace

struct ClientHFSMDebugger::Graph {
    ImFlow::ImNodeFlow editor;
    std::unordered_map<HFSMNodeID, std::shared_ptr<HFSMNodeView>> views;
    std::vector<HFSMNodeID> node_ids;
};

ClientHFSMDebugger::ClientHFSMDebugger() = default;

ClientHFSMDebugger::~ClientHFSMDebugger() = default;

void ClientHFSMDebugger::ToggleVisible(LogicEntity entity) {
    if (auto it = m_visible.find(entity); it != m_visible.end()) {
        m_visible.erase(it);
        m_graphs.erase(entity);
    } else {
        m_visible.emplace(entity);
    }
}

ClientHFSMDebugger::Graph& ClientHFSMDebugger::GetGraph(LogicEntity entity) {
    auto& graph = m_graphs[entity];
    if (!graph) {
        graph = std::make_unique<Graph>();
    }
    return *graph;
}

void ClientHFSMDebugger::RenderGraph(Graph& graph, HFSMComponent& component) {
    const std::vector<HFSMNodeID> ids = CollectNodeIDs(component);
    if (ids != graph.node_ids) {
        graph.node_ids = ids;
        graph.editor.getNodes().clear();
        graph.views.clear();

        // Tree layout: leaves are laid out left to right, parents are centered
        // over their children.
        std::unordered_map<HFSMNodeID, ImVec2> positions;
        float cursor = 0.f;
        std::function<void(HFSMNode*, int)> place = [&](HFSMNode* node,
                                                        int depth) {
            if (!node) {
                return;
            }

            const auto& children = node->GetChildren();
            if (children.empty()) {
                positions[node->GetID()] =
                    ImVec2(cursor, static_cast<float>(depth) * kNodeSpacingY);
                cursor += kNodeSpacingX;
                return;
            }

            for (HFSMNode* child : children) {
                place(child, depth + 1);
            }

            const float x =
                (positions[children.front()->GetID()].x +
                 positions[children.back()->GetID()].x) *
                0.5f;
            positions[node->GetID()] =
                ImVec2(x, static_cast<float>(depth) * kNodeSpacingY);
        };

        std::vector<HFSMNode*> roots;
        for (const auto& [id, node] : component.GetNodes()) {
            if (!node->GetParent()) {
                roots.push_back(node.get());
            }
        }
        std::sort(roots.begin(), roots.end(),
                  [](const HFSMNode* a, const HFSMNode* b) {
                      return a->GetID() < b->GetID();
                  });
        for (HFSMNode* root : roots) {
            place(root, 0);
        }

        for (HFSMNodeID id : graph.node_ids) {
            HFSMNode* node = component.GetNode(id);
            TL_CONTINUE_IF_NULL(node);
            graph.views[id] = graph.editor.addNode<HFSMNodeView>(
                positions[id], id, node->GetName());
        }

        for (HFSMNodeID id : graph.node_ids) {
            HFSMNode* node = component.GetNode(id);
            TL_CONTINUE_IF_NULL(node);

            auto view_it = graph.views.find(id);
            if (view_it == graph.views.end()) {
                continue;
            }
            ImFlow::Pin* out = view_it->second->outPin("");
            if (!out) {
                continue;
            }

            for (HFSMNode* child : node->GetChildren()) {
                auto child_it = graph.views.find(child->GetID());
                if (child_it == graph.views.end()) {
                    continue;
                }
                ImFlow::Pin* in = child_it->second->inPin("");
                if (in) {
                    out->createLink(in);
                }
            }
        }
    }

    // Highlight the node that is currently running.
    const HFSMNode* current = component.GetCurrentNode();
    for (auto& [id, view] : graph.views) {
        const bool is_current = current && current->GetID() == id;
        view->getStyle()->header_bg =
            is_current ? kCurrentHeaderColor : kIdleHeaderColor;
    }

    graph.editor.update();
}

bool ClientHFSMDebugger::RenderWindow(LogicEntity entity,
                                      HFSMComponent& component) {
    std::string title = component.GetAssetName();
    if (title.empty()) {
        title = "HFSM";
    }
    // Keep the displayed name but make the window ID unique per entity so two
    // entities sharing an asset don't collide.
    title += "###hfsm_debug_";
    title += std::to_string(
        static_cast<std::underlying_type_t<LogicEntity>>(entity));

    bool open = true;
    ImGui::SetNextWindowSize(ImVec2(640.f, 420.f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(title.c_str(), &open)) {
        RenderGraph(GetGraph(entity), component);
    }
    ImGui::End();

    return open;
}

void ClientHFSMDebugger::Render() {
    if (!COMMON_CONTEXT.m_hfsm_manager) {
        return;
    }

    for (auto it = m_visible.begin(); it != m_visible.end();) {
        const LogicEntity entity = *it;
        HFSMComponent* component = COMMON_CONTEXT.m_hfsm_manager->Get(entity);
        if (!component) {
            m_graphs.erase(entity);
            it = m_visible.erase(it);
            continue;
        }

        if (RenderWindow(entity, *component)) {
            ++it;
        } else {
            m_graphs.erase(entity);
            it = m_visible.erase(it);
        }
    }
}
