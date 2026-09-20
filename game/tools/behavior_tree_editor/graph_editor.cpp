#include "graph_editor.hpp"

#include "ImNodeFlow.h"
#include "common/dialog.hpp"
#include "imgui.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <functional>
#include <string>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

constexpr float kSpacingX = 460.0f;
constexpr float kSpacingY = 240.0f;

const char* kNodeTypeNames[] = {
    "Sequence", "Selector", "Parallel",  "Inverter", "Succeeder",
    "Failer",   "Repeat",   "UntilFail", "Wait",     "Action",
    "Condition",
};
constexpr int kNodeTypeCount = 11;

bool IsComposite(BehaviorTreeNodeType type) {
    return type == BehaviorTreeNodeType::Sequence ||
           type == BehaviorTreeNodeType::Selector ||
           type == BehaviorTreeNodeType::Parallel;
}

bool IsDecorator(BehaviorTreeNodeType type) {
    return type == BehaviorTreeNodeType::Inverter ||
           type == BehaviorTreeNodeType::Succeeder ||
           type == BehaviorTreeNodeType::Failer ||
           type == BehaviorTreeNodeType::Repeat ||
           type == BehaviorTreeNodeType::UntilFail;
}

bool IsLeaf(BehaviorTreeNodeType type) {
    return type == BehaviorTreeNodeType::Wait ||
           type == BehaviorTreeNodeType::Action ||
           type == BehaviorTreeNodeType::Condition;
}

ImU32 HeaderColor(BehaviorTreeNodeType type) {
    if (IsComposite(type)) {
        return IM_COL32(180, 120, 60, 255);
    }
    if (IsDecorator(type)) {
        return IM_COL32(80, 130, 170, 255);
    }
    return IM_COL32(70, 150, 90, 255);
}

// Node body widgets are drawn inside the editor window, so an unbounded
// `ImGui::Separator()` would stretch across the whole window. These helpers
// keep every row and separator within a fixed content width.
constexpr float kFieldWidth = 150.0f;
constexpr float kContentWidth = 230.0f;

void LabeledRow(const char* label, float field_width = kFieldWidth) {
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(field_width);
}

void BoundedSeparator() {
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(pos.x, pos.y), ImVec2(pos.x + kContentWidth, pos.y),
        ImGui::GetColorU32(ImGuiCol_Separator));
    ImGui::Dummy(ImVec2(kContentWidth, 1.0f));
}

void SameLineButtonTooltip(const char* text) {
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", text);
    }
}

/**
 * Opens the native file dialog to pick a Lua script. The chosen file is
 * converted to a path relative to the working directory (the project root),
 * matching how scripts are stored in the asset.
 */
bool SelectLuaScript(Path& out_path) {
    FileDialog dialog{FileDialog::Type::OpenFile};
    dialog.SetTitle("Select Lua Script");
    dialog.AddFilter("Lua Script (*.lua)", "lua");

    std::filesystem::path default_dir =
        std::filesystem::current_path() / "scripts";
    if (!std::filesystem::exists(default_dir)) {
        default_dir = std::filesystem::current_path();
    }
    dialog.SetDefaultFolder(Path(default_dir.string()));
    dialog.Open();

    const auto& files = dialog.GetSelectedFiles();
    if (files.empty()) {
        return false;
    }

    std::error_code err;
    auto relative = std::filesystem::relative(files[0].string(),
                                              std::filesystem::current_path(),
                                              err);
    std::string path = err ? files[0].string() : relative.string();
    std::replace(path.begin(), path.end(), '\\', '/');
    out_path = Path(path);
    return true;
}

}  // namespace

struct BehaviorTreeGraphEditorImpl;

class BTNodeView : public ImFlow::BaseNode {
public:
    BTNodeView(uint32_t id, BehaviorTreeDefinition* tree,
               BehaviorTreeNodeDefinition* node,
               BehaviorTreeGraphEditorImpl* owner);

    void draw() override;

    [[nodiscard]] uint32_t GetNodeID() const { return m_id; }
    [[nodiscard]] BehaviorTreeNodeDefinition* GetNode() const { return m_node; }

    bool CanConnect(ImFlow::Pin* a, ImFlow::Pin* b) const;

private:
    uint32_t m_id;
    BehaviorTreeDefinition* m_tree;
    BehaviorTreeNodeDefinition* m_node;
    BehaviorTreeGraphEditorImpl* m_owner;
};

struct BehaviorTreeGraphEditorImpl {
    ImFlow::ImNodeFlow editor;
    BehaviorTreeDefinition* tree = nullptr;
    std::unordered_map<uint32_t, std::shared_ptr<BTNodeView>> views;
    std::unordered_map<uint32_t, ImVec2> positions;
    bool rebuild = true;
    bool dirty = false;

    uint32_t NextID() const {
        uint32_t max_id = 0;
        for (const auto& node : tree->m_nodes) {
            max_id = std::max(max_id, node.m_id + 1);
        }
        return max_id;
    }

    BehaviorTreeNodeDefinition* Find(uint32_t id) {
        for (auto& node : tree->m_nodes) {
            if (node.m_id == id) {
                return &node;
            }
        }
        return nullptr;
    }

    bool CanReach(uint32_t from, uint32_t target) const {
        if (from == target) {
            return true;
        }
        const BehaviorTreeNodeDefinition* node = nullptr;
        for (const auto& n : tree->m_nodes) {
            if (n.m_id == from) {
                node = &n;
                break;
            }
        }
        if (!node) {
            return false;
        }
        for (uint32_t child : node->m_children) {
            if (CanReach(child, target)) {
                return true;
            }
        }
        return false;
    }

    std::pair<uint32_t, uint32_t> LinkEnds(
        const std::shared_ptr<ImFlow::Link>& link) const {
        ImFlow::Pin* left = link->left();
        ImFlow::Pin* right = link->right();
        if (left->getType() == ImFlow::PinType_Input) {
            std::swap(left, right);
        }
        auto* parent = static_cast<BTNodeView*>(left->getParent());
        auto* child = static_cast<BTNodeView*>(right->getParent());
        if (!parent || !child) {
            return {0, 0};
        }
        return {parent->GetNodeID(), child->GetNodeID()};
    }

    void AddNodeAt(BehaviorTreeNodeType type, const ImVec2& grid_pos) {
        BehaviorTreeNodeDefinition node;
        node.m_id = NextID();
        node.m_name = kNodeTypeNames[static_cast<int>(type)];
        node.m_type = type;
        tree->m_nodes.push_back(node);
        positions[node.m_id] = grid_pos;
        rebuild = true;
        dirty = true;
    }

    void Rebuild() {
        editor.getNodes().clear();
        views.clear();
        rebuild = false;

        if (!tree || tree->m_nodes.empty()) {
            return;
        }

        std::unordered_map<uint32_t, BehaviorTreeNodeDefinition*>
            by_id;
        std::unordered_set<uint32_t> child_ids;
        for (auto& node : tree->m_nodes) {
            by_id[node.m_id] = &node;
            for (uint32_t child : node.m_children) {
                child_ids.insert(child);
            }
        }

        // Auto-layout nodes that have no position yet.
        std::unordered_map<uint32_t, ImVec2> layout;
        if (auto computed = ComputeLayout(child_ids); !computed.empty()) {
            layout = std::move(computed);
        }

        for (auto& node : tree->m_nodes) {
            ImVec2 pos;
            if (auto it = positions.find(node.m_id); it != positions.end()) {
                pos = it->second;
            } else if (auto it = layout.find(node.m_id); it != layout.end()) {
                pos = it->second;
            } else {
                pos = ImVec2(0.0f, 0.0f);
            }
            auto view = editor.addNode<BTNodeView>(pos, node.m_id, tree, &node,
                                                   this);
            views[node.m_id] = view;
        }

        for (auto& node : tree->m_nodes) {
            auto view_it = views.find(node.m_id);
            if (view_it == views.end() || !view_it->second) {
                continue;
            }
            ImFlow::Pin* out = view_it->second->outPin("");
            if (!out) {
                continue;
            }
            for (uint32_t child : node.m_children) {
                auto child_it = views.find(child);
                if (child_it == views.end() || !child_it->second) {
                    continue;
                }
                ImFlow::Pin* in = child_it->second->inPin("");
                if (in) {
                    out->createLink(in);
                }
            }
        }
    }

    std::unordered_map<uint32_t, ImVec2> ComputeLayout(
        const std::unordered_set<uint32_t>& child_ids) {
        std::unordered_map<uint32_t, ImVec2> result;
        float next_x = 0.0f;

        std::function<void(BehaviorTreeNodeDefinition&, int)> place =
            [&](BehaviorTreeNodeDefinition& node, int depth) {
                const float y = static_cast<float>(depth) * kSpacingY;
                if (node.m_children.empty()) {
                    result[node.m_id] = ImVec2(next_x, y);
                    next_x += kSpacingX;
                    return;
                }
                const float first_x = next_x;
                for (uint32_t child : node.m_children) {
                    BehaviorTreeNodeDefinition* child_node = Find(child);
                    if (child_node) {
                        place(*child_node, depth + 1);
                    }
                }
                const float center_x = (first_x + next_x - kSpacingX) * 0.5f;
                result[node.m_id] = ImVec2(center_x, y);
            };

        for (auto& node : tree->m_nodes) {
            if (child_ids.find(node.m_id) == child_ids.end()) {
                place(node, 0);
            }
        }
        return result;
    }

    void Reconcile() {
        // Edges currently present in the graph.
        std::unordered_map<uint32_t, std::vector<uint32_t>>
            edges;
        for (const auto& weak : editor.getLinks()) {
            auto link = weak.lock();
            if (!link) {
                continue;
            }
            auto [parent, child] = LinkEnds(link);
            if (parent == 0 && child == 0) {
                continue;
            }
            edges[parent].push_back(child);
        }

        // Nodes still alive in the graph.
        std::unordered_set<uint32_t> alive;
        for (auto& [uid, base] : editor.getNodes()) {
            auto* view = static_cast<BTNodeView*>(base.get());
            alive.insert(view->GetNodeID());
        }

        // Remove nodes deleted through the graph.
        std::unordered_set<uint32_t> removed;
        for (auto& [id, view] : views) {
            if (alive.find(id) == alive.end()) {
                removed.insert(id);
            }
        }
        if (!removed.empty()) {
            for (auto& node : tree->m_nodes) {
                node.m_children.erase(
                    std::remove_if(node.m_children.begin(),
                                   node.m_children.end(),
                                   [&](uint32_t id) {
                                       return removed.count(id) > 0;
                                   }),
                    node.m_children.end());
            }
            tree->m_nodes.erase(
                std::remove_if(tree->m_nodes.begin(), tree->m_nodes.end(),
                               [&](const BehaviorTreeNodeDefinition& node) {
                                   return removed.count(node.m_id) > 0;
                               }),
                tree->m_nodes.end());
            for (uint32_t id : removed) {
                views.erase(id);
                positions.erase(id);
            }
            dirty = true;
            rebuild = true;
            return;
        }

        // Sync the ordered children list with the graph edges.
        for (auto& node : tree->m_nodes) {
            const auto edge_it = edges.find(node.m_id);
            const std::vector<uint32_t> empty;
            const auto& node_edges =
                edge_it == edges.end() ? empty : edge_it->second;

            std::vector<uint32_t> children;
            for (uint32_t child : node.m_children) {
                if (std::find(node_edges.begin(), node_edges.end(), child) !=
                    node_edges.end()) {
                    children.push_back(child);
                }
            }
            for (uint32_t child : node_edges) {
                if (std::find(children.begin(), children.end(), child) ==
                    children.end()) {
                    children.push_back(child);
                }
            }

            if (children != node.m_children) {
                node.m_children = children;
                dirty = true;
            }
        }
    }
};

BTNodeView::BTNodeView(uint32_t id, BehaviorTreeDefinition* tree,
                       BehaviorTreeNodeDefinition* node,
                       BehaviorTreeGraphEditorImpl* owner)
    : m_id(id), m_tree(tree), m_node(node), m_owner(owner) {
    setTitle(kNodeTypeNames[static_cast<int>(m_node->m_type)]);
    addIN<int>("", 0, [this](ImFlow::Pin* a, ImFlow::Pin* b) {
        return CanConnect(a, b);
    });
    (void)addOUT<int>("");
}

bool BTNodeView::CanConnect(ImFlow::Pin* a, ImFlow::Pin* b) const {
    ImFlow::Pin* in = a;
    ImFlow::Pin* out = b;
    if (in->getType() != ImFlow::PinType_Input) {
        std::swap(in, out);
    }
    if (in->isConnected()) {
        return false;
    }

    auto* parent = static_cast<BTNodeView*>(out->getParent());
    auto* child = static_cast<BTNodeView*>(in->getParent());
    if (!parent || !child || parent == child) {
        return false;
    }

    const BehaviorTreeNodeDefinition* parent_node = parent->GetNode();
    if (IsLeaf(parent_node->m_type)) {
        return false;
    }
    if (IsDecorator(parent_node->m_type) &&
        parent_node->m_children.size() >= 1) {
        return false;
    }
    if (m_owner->CanReach(child->GetNodeID(), parent->GetNodeID())) {
        return false;
    }
    return true;
}

void BTNodeView::draw() {
    getStyle()->header_bg = HeaderColor(m_node->m_type);
    setTitle(m_node->m_name.empty()
                 ? kNodeTypeNames[static_cast<int>(m_node->m_type)]
                 : m_node->m_name);

    ImGui::PushID(this);

    char name_buffer[128];
    std::snprintf(name_buffer, sizeof(name_buffer), "%s",
                  m_node->m_name.c_str());
    LabeledRow("name");
    if (ImGui::InputText("##name", name_buffer, sizeof(name_buffer))) {
        m_node->m_name = name_buffer;
        m_owner->dirty = true;
    }

    int type = static_cast<int>(m_node->m_type);
    LabeledRow("type");
    if (ImGui::Combo("##type", &type, kNodeTypeNames, kNodeTypeCount)) {
        const auto new_type = static_cast<BehaviorTreeNodeType>(type);
        m_node->m_type = new_type;
        m_owner->dirty = true;
        if (IsLeaf(new_type) && !m_node->m_children.empty()) {
            m_node->m_children.clear();
            m_owner->rebuild = true;
        }
        if (IsDecorator(new_type) && m_node->m_children.size() > 1) {
            m_node->m_children.resize(1);
            m_owner->rebuild = true;
        }
    }

    if (m_node->m_type == BehaviorTreeNodeType::Wait) {
        LabeledRow("time");
        if (ImGui::DragFloat("##time", &m_node->m_time, 0.05f, 0.0f, 100.0f)) {
            m_owner->dirty = true;
        }
    } else if (m_node->m_type == BehaviorTreeNodeType::Repeat) {
        LabeledRow("count");
        if (ImGui::DragInt("##count", &m_node->m_count, 1, 0, 100)) {
            m_owner->dirty = true;
        }
    } else if (m_node->m_type == BehaviorTreeNodeType::Parallel) {
        int success = static_cast<int>(m_node->m_success_policy);
        const char* success_items[] = {"All", "Any"};
        LabeledRow("success");
        if (ImGui::Combo("##success", &success, success_items, 2)) {
            m_node->m_success_policy =
                static_cast<ParallelSuccessPolicy>(success);
            m_owner->dirty = true;
        }
        int failure = static_cast<int>(m_node->m_failure_policy);
        const char* failure_items[] = {"Any", "All"};
        LabeledRow("failure");
        if (ImGui::Combo("##failure", &failure, failure_items, 2)) {
            m_node->m_failure_policy =
                static_cast<ParallelFailurePolicy>(failure);
            m_owner->dirty = true;
        }
    } else if (m_node->m_type == BehaviorTreeNodeType::Action ||
               m_node->m_type == BehaviorTreeNodeType::Condition) {
        char script_buffer[1024];
        std::snprintf(script_buffer, sizeof(script_buffer), "%s",
                      m_node->m_script.string().c_str());

        // Size the read-only box to the whole path so it is fully visible.
        const float text_width = ImGui::CalcTextSize(script_buffer).x;
        const float box_width =
            std::max(kFieldWidth, text_width +
                                      ImGui::GetStyle().FramePadding.x * 2.0f +
                                      4.0f);

        LabeledRow("script", box_width);
        ImGui::InputTextWithHint("##script", "(no script)", script_buffer,
                                 sizeof(script_buffer),
                                 ImGuiInputTextFlags_ReadOnly);
        ImGui::SameLine();
        const bool has_script = !m_node->m_script.empty();
        if (ImGui::Button(has_script ? "Change..." : "Select...")) {
            Path selected;
            if (SelectLuaScript(selected)) {
                m_node->m_script = selected;
                m_owner->dirty = true;
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Choose a .lua script for this leaf");
        }
    }

    // Ordered children editor (order matters for Sequence/Selector/Parallel).
    if (!m_node->m_children.empty()) {
        BoundedSeparator();
        ImGui::TextUnformatted("children");
        for (size_t i = 0; i < m_node->m_children.size(); ++i) {
            const uint32_t child_id = m_node->m_children[i];
            BehaviorTreeNodeDefinition* child = m_owner->Find(child_id);
            const char* child_name =
                child ? child->m_name.c_str() : "(missing)";

            ImGui::PushID(static_cast<int>(i));
            ImGui::Text("%llu. %s", static_cast<unsigned long long>(i),
                        child_name);
            ImGui::SameLine();

            const bool can_up = i > 0;
            const bool can_down = i + 1 < m_node->m_children.size();

            ImGui::BeginDisabled(!can_up);
            if (ImGui::SmallButton("^")) {
                std::swap(m_node->m_children[i], m_node->m_children[i - 1]);
                m_owner->dirty = true;
            }
            ImGui::EndDisabled();
            SameLineButtonTooltip("Move this child up");

            ImGui::SameLine();
            ImGui::BeginDisabled(!can_down);
            if (ImGui::SmallButton("v")) {
                std::swap(m_node->m_children[i], m_node->m_children[i + 1]);
                m_owner->dirty = true;
            }
            ImGui::EndDisabled();
            SameLineButtonTooltip("Move this child down");

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.62f, 0.22f, 0.22f, 1.0f));
            const bool remove_child = ImGui::SmallButton("x");
            ImGui::PopStyleColor();
            SameLineButtonTooltip("Remove this child");

            if (remove_child) {
                m_node->m_children.erase(m_node->m_children.begin() +
                                         static_cast<long>(i));
                m_owner->dirty = true;
                m_owner->rebuild = true;
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
        }
    }

    ImGui::PopID();
}

BehaviorTreeGraphEditor::BehaviorTreeGraphEditor()
    : m_impl(std::make_unique<BehaviorTreeGraphEditorImpl>()) {}

BehaviorTreeGraphEditor::~BehaviorTreeGraphEditor() = default;

void BehaviorTreeGraphEditor::RequestRebuild() {
    m_impl->rebuild = true;
}

void BehaviorTreeGraphEditor::AutoLayout() {
    m_impl->positions.clear();
    m_impl->rebuild = true;
}

void BehaviorTreeGraphEditor::Reset() {
    m_impl.reset();
}

void BehaviorTreeGraphEditor::Update(BehaviorTreeDefinition& tree,
                                     bool& dirty) {
    BehaviorTreeGraphEditorImpl& impl = *m_impl;
    impl.tree = &tree;
    impl.dirty = false;

    for (auto& [id, view] : impl.views) {
        if (view) {
            impl.positions[id] = view->getPos();
        }
    }

    if (impl.rebuild) {
        impl.Rebuild();
    }

    BehaviorTreeGraphEditorImpl* self = m_impl.get();
    self->editor.rightClickPopUpContent([self](ImFlow::BaseNode* /*node*/) {
        if (ImGui::BeginMenu("Add Node")) {
            for (int i = 0; i < kNodeTypeCount; ++i) {
                if (ImGui::MenuItem(kNodeTypeNames[i])) {
                    self->AddNodeAt(
                        static_cast<BehaviorTreeNodeType>(i),
                        self->editor.screen2grid(ImGui::GetMousePos()));
                }
            }
            ImGui::EndMenu();
        }
    });

    impl.editor.update();

    for (auto& [id, view] : impl.views) {
        if (view) {
            impl.positions[id] = view->getPos();
        }
    }

    // A rebuild was requested while drawing (add/remove/reorder child, type
    // change). Skip reconcile this frame: the graph will be recreated from the
    // model on the next Update, and reconciling now would re-add a child that
    // the user just removed from the ordered list.
    if (!impl.rebuild) {
        impl.Reconcile();
    }

    if (impl.dirty) {
        dirty = true;
    }
}
