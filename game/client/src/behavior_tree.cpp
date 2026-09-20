#include "client/behavior_tree.hpp"

#include "common/behavior_tree.hpp"
#include "common/context.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"

#include "ImNodeFlow.h"
#include "imgui.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

constexpr float kNodeSpacingX = 220.f;
constexpr float kNodeSpacingY = 150.f;
constexpr ImU32 kRunningHeaderColor = IM_COL32(224, 184, 58, 255);
constexpr ImU32 kIdleHeaderColor = IM_COL32(71, 142, 173, 255);

class BTNodeView : public ImFlow::BaseNode {
public:
    BTNodeView(BehaviorTreeNodeID id, std::string name) : m_bt_id(id) {
        setTitle(std::move(name));
        addIN<int>("", 0, ImFlow::ConnectionFilter::None());
        (void)addOUT<int>("");
    }

    [[nodiscard]] BehaviorTreeNodeID GetBTNodeID() const { return m_bt_id; }

private:
    BehaviorTreeNodeID m_bt_id;
};

std::vector<BehaviorTreeNodeID> CollectNodeIDs(
    const BehaviorTreeComponent& component) {
    std::vector<BehaviorTreeNodeID> ids;
    ids.reserve(component.GetNodes().size());
    for (const auto& [id, node] : component.GetNodes()) {
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

}  // namespace

struct ClientBTDebugger::Graph {
    ImFlow::ImNodeFlow editor;
    std::unordered_map<BehaviorTreeNodeID, std::shared_ptr<BTNodeView>> views;
    std::vector<BehaviorTreeNodeID> node_ids;
};

ClientBTDebugger::ClientBTDebugger() = default;

ClientBTDebugger::~ClientBTDebugger() = default;

void ClientBTDebugger::ToggleVisible(LogicEntity entity) {
    if (auto it = m_visible.find(entity); it != m_visible.end()) {
        m_visible.erase(it);
        m_graphs.erase(entity);
    } else {
        m_visible.emplace(entity);
    }
}

ClientBTDebugger::Graph& ClientBTDebugger::GetGraph(LogicEntity entity) {
    auto& graph = m_graphs[entity];
    if (!graph) {
        graph = std::make_unique<Graph>();
    }
    return *graph;
}

void ClientBTDebugger::RenderGraph(Graph& graph,
                                   BehaviorTreeComponent& component) {
    const std::vector<BehaviorTreeNodeID> ids = CollectNodeIDs(component);
    if (ids != graph.node_ids) {
        graph.node_ids = ids;
        graph.editor.getNodes().clear();
        graph.views.clear();

        // Tree layout: leaves are laid out left to right, parents are centered
        // over their children.
        std::unordered_map<BehaviorTreeNodeID, ImVec2> positions;
        float cursor = 0.f;
        std::function<void(BehaviorTreeNode*, int)> place =
            [&](BehaviorTreeNode* node, int depth) {
                if (!node) {
                    return;
                }

                const auto& children = node->GetChildren();
                if (children.empty()) {
                    positions[node->GetID()] = ImVec2(
                        cursor, static_cast<float>(depth) * kNodeSpacingY);
                    cursor += kNodeSpacingX;
                    return;
                }

                for (BehaviorTreeNode* child : children) {
                    place(child, depth + 1);
                }

                const float x = (positions[children.front()->GetID()].x +
                                 positions[children.back()->GetID()].x) *
                                0.5f;
                positions[node->GetID()] = ImVec2(
                    x, static_cast<float>(depth) * kNodeSpacingY);
            };

        std::unordered_set<BehaviorTreeNodeID> child_ids;
        for (const auto& [id, node] : component.GetNodes()) {
            for (BehaviorTreeNode* child : node->GetChildren()) {
                child_ids.insert(child->GetID());
            }
        }

        std::vector<BehaviorTreeNode*> roots;
        for (const auto& [id, node] : component.GetNodes()) {
            if (child_ids.find(id) == child_ids.end()) {
                roots.push_back(node.get());
            }
        }
        std::sort(roots.begin(), roots.end(),
                  [](const BehaviorTreeNode* a, const BehaviorTreeNode* b) {
                      return a->GetID() < b->GetID();
                  });
        for (BehaviorTreeNode* root : roots) {
            place(root, 0);
        }

        for (BehaviorTreeNodeID id : graph.node_ids) {
            BehaviorTreeNode* node = component.GetNode(id);
            TL_CONTINUE_IF_NULL(node);
            graph.views[id] = graph.editor.addNode<BTNodeView>(
                positions[id], id, node->GetName());
        }

        for (BehaviorTreeNodeID id : graph.node_ids) {
            BehaviorTreeNode* node = component.GetNode(id);
            TL_CONTINUE_IF_NULL(node);

            auto view_it = graph.views.find(id);
            if (view_it == graph.views.end()) {
                continue;
            }
            ImFlow::Pin* out = view_it->second->outPin("");
            if (!out) {
                continue;
            }

            for (BehaviorTreeNode* child : node->GetChildren()) {
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

    // Highlight every node on the currently running path.
    for (auto& [id, view] : graph.views) {
        BehaviorTreeNode* node = component.GetNode(id);
        const bool running = node && node->IsRunning();
        view->getStyle()->header_bg =
            running ? kRunningHeaderColor : kIdleHeaderColor;
    }

    graph.editor.update();
}

bool ClientBTDebugger::RenderWindow(LogicEntity entity,
                                    BehaviorTreeComponent& component) {
    std::string title = component.GetAssetName();
    if (title.empty()) {
        title = "BehaviorTree";
    }

    title += "###bt_debug_";
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

void ClientBTDebugger::Render() {
    if (!COMMON_CONTEXT.m_behavior_tree_manager) {
        return;
    }

    for (auto it = m_visible.begin(); it != m_visible.end();) {
        const LogicEntity entity = *it;
        BehaviorTreeComponent* component =
            COMMON_CONTEXT.m_behavior_tree_manager->Get(entity);
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
