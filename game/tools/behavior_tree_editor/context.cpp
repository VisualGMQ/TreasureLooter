#include "context.hpp"

#include "client/animation_player.hpp"
#include "client/controller.hpp"
#include "client/debug_panel.hpp"
#include "client/detour.hpp"
#include "client/draw_order.hpp"
#include "client/hfsm.hpp"
#include "client/input/finger_touch.hpp"
#include "client/input/gamepad.hpp"
#include "client/input/input.hpp"
#include "client/input/keyboard.hpp"
#include "client/input/mouse.hpp"
#include "client/renderer.hpp"
#include "client/sprite.hpp"
#include "client/tilemap_layer_collision_component.hpp"
#include "client/tilemap_render_component.hpp"
#include "client/ui.hpp"
#include "client/window.hpp"
#include "common/dialog.hpp"
#include "common/log.hpp"
#include "imgui.h"
#include "lyra/lyra.hpp"
#include "schema/serialize/behavior_tree.hpp"

#include <algorithm>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

constexpr std::string_view kAssetExtension = ".bt_definition.xml";

bool HasAssetExtension(const std::string& filename) {
    if (filename.size() < kAssetExtension.size()) {
        return false;
    }
    return filename.compare(filename.size() - kAssetExtension.size(),
                            kAssetExtension.size(), kAssetExtension) == 0;
}

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

void RunValidation(const BehaviorTreeDefinition& tree,
                   std::vector<std::string>& out) {
    std::unordered_map<uint32_t, int> id_count;
    std::unordered_set<uint32_t> ids;
    std::unordered_set<uint32_t> child_ids;
    for (const auto& node : tree.m_nodes) {
        ids.insert(node.m_id);
        ++id_count[node.m_id];
        for (uint32_t child : node.m_children) {
            child_ids.insert(child);
        }
    }

    for (const auto& [id, count] : id_count) {
        if (count > 1) {
            out.push_back("Duplicate node id " + std::to_string(id));
        }
    }

    int roots = 0;
    for (const auto& node : tree.m_nodes) {
        if (child_ids.find(node.m_id) == child_ids.end()) {
            ++roots;
        }
    }
    if (tree.m_nodes.empty()) {
        out.push_back("Tree has no nodes");
    } else if (roots != 1) {
        out.push_back("Tree must have exactly one root (found " +
                      std::to_string(roots) + ")");
    }

    for (const auto& node : tree.m_nodes) {
        const std::string label =
            node.m_name.empty() ? ("id " + std::to_string(node.m_id))
                                : node.m_name;
        for (uint32_t child : node.m_children) {
            if (ids.find(child) == ids.end()) {
                out.push_back(label + " references missing child " +
                              std::to_string(child));
            }
        }
        if (IsComposite(node.m_type) && node.m_children.empty()) {
            out.push_back(label + " (composite) has no children");
        }
        if (IsDecorator(node.m_type) && node.m_children.size() != 1) {
            out.push_back(label + " (decorator) must have exactly 1 child");
        }
        if (IsLeaf(node.m_type) && !node.m_children.empty()) {
            out.push_back(label + " (leaf) must not have children");
        }
        if ((node.m_type == BehaviorTreeNodeType::Action ||
             node.m_type == BehaviorTreeNodeType::Condition) &&
            node.m_script.empty()) {
            out.push_back(label + " must reference a script");
        }
    }
}

}  // namespace

void BehaviorTreeEditorContext::Init() {
    if (!instance) {
        instance = std::unique_ptr<BehaviorTreeEditorContext>(
            new BehaviorTreeEditorContext);
    } else {
        LOGW("inited context singleton twice!");
    }
}

void BehaviorTreeEditorContext::Destroy() {
    instance.reset();
}

BehaviorTreeEditorContext& BehaviorTreeEditorContext::GetInst() {
    return static_cast<BehaviorTreeEditorContext&>(*instance);
}

void BehaviorTreeEditorContext::Initialize(int argc, char** argv) {
    ToolContext::Initialize(argc, argv);

    m_window->SetTitle("TreasureLooter BehaviorTree Editor");
    m_window->Resize({1100, 720});
    parseCmdArgs(argc, argv);
}

void BehaviorTreeEditorContext::Shutdown() {
    // Destroy the ImNodeFlow editor (and its embedded ImGui context) before
    // the base context shuts the main ImGui context down, otherwise the
    // embedded context unregisters fonts from an already-freed atlas.
    m_graph.Reset();
    ToolContext::Shutdown();
}

void BehaviorTreeEditorContext::parseCmdArgs(int argc, char** argv) {
    std::filesystem::path filename;
    auto cli = lyra::cli() | lyra::opt(filename, "filename")["--filename"];
    lyra::parse_result result = cli.parse({argc, argv});

    if (!result) {
        LOGE("Command line parse failed: {}", result.message());
    }

    if (std::filesystem::is_regular_file(filename)) {
        load(filename);
    } else if (!filename.empty()) {
        LOGW("behavior tree editor cannot open '{}'", filename.string());
    }
}

void BehaviorTreeEditorContext::load(const Path& filename) {
    auto result = LoadAsset<BehaviorTreeDefinition>(filename);
    if (!result) {
        LOGE("load behavior tree {} failed", filename);
        return;
    }
    m_tree = std::move(*result.m_payload);
    m_uuid = result.m_uuid;
    m_path = filename;
    m_dirty = false;
    m_graph.RequestRebuild();
    updateTitle();
}

void BehaviorTreeEditorContext::newTree() {
    m_tree = BehaviorTreeDefinition{};
    m_uuid = UUIDv4::CreateV4();
    m_path = Path();
    m_dirty = false;
    m_graph.RequestRebuild();
    updateTitle();
}

void BehaviorTreeEditorContext::openFile() {
    FileDialog dialog{FileDialog::Type::OpenFile};
    dialog.SetTitle("Open Behavior Tree");
    dialog.AddFilter("Behavior Tree (*.bt_definition.xml)", "bt_definition.xml");
    dialog.SetDefaultFolder(std::filesystem::current_path().string());
    dialog.Open();

    const auto& files = dialog.GetSelectedFiles();
    if (!files.empty()) {
        load(files[0]);
    }
}

void BehaviorTreeEditorContext::save() {
    if (m_path.empty()) {
        saveAs();
        return;
    }
    SaveAsset(m_uuid, m_tree, m_path);
    m_dirty = false;
    updateTitle();
}

void BehaviorTreeEditorContext::saveAs() {
    FileDialog dialog{FileDialog::Type::SaveFile};
    dialog.SetTitle("Save Behavior Tree As");
    dialog.AddFilter("Behavior Tree (*.bt_definition.xml)", "bt_definition.xml");
    dialog.SetDefaultFolder(std::filesystem::current_path().string());
    dialog.Open();

    const auto& files = dialog.GetSelectedFiles();
    if (files.empty()) {
        return;
    }
    Path filename = files[0];
    if (!HasAssetExtension(filename.string())) {
        filename = Path(filename.string() + std::string(kAssetExtension));
    }
    SaveAsset(m_uuid, m_tree, filename);
    m_path = filename;
    m_dirty = false;
    updateTitle();
}

void BehaviorTreeEditorContext::updateTitle() {
    std::string title = "TreasureLooter BehaviorTree Editor - ";
    title += m_path.empty() ? "[No Name]" : m_path.string();
    if (m_dirty) {
        title += " *";
    }
    m_window->SetTitle(title);
}

void BehaviorTreeEditorContext::showMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                newTree();
            }
            if (ImGui::MenuItem("Open...")) {
                openFile();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save")) {
                save();
            }
            if (ImGui::MenuItem("Save As...")) {
                saveAs();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void BehaviorTreeEditorContext::showSidePanel() {
    if (ImGui::Button("Auto Layout")) {
        m_graph.AutoLayout();
    }

    ImGui::SeparatorText("Validation");
    std::vector<std::string> messages;
    RunValidation(m_tree, messages);
    if (messages.empty()) {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "OK");
    } else {
        for (const auto& message : messages) {
            ImGui::TextWrapped("- %s", message.c_str());
        }
    }

    ImGui::SeparatorText("Help");
    ImGui::TextWrapped(
        "Right-click the canvas to add a node. Drag from a node's right pin to "
        "another node's left pin to connect. Select a node and press Delete to "
        "remove it. Edit fields inside each node; reorder children with ^ / v.");
}

void BehaviorTreeEditorContext::update() {
    showMainMenu();

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("##bt_editor_full", nullptr, flags)) {
        ImGui::BeginChild("##bt_side", ImVec2(340.0f, 0.0f), true);
        showSidePanel();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##bt_graph", ImVec2(0.0f, 0.0f), true);
        m_graph.Update(m_tree, m_dirty);
        ImGui::EndChild();
    }
    ImGui::End();

    if (m_dirty) {
        updateTitle();
    }
}
