#pragma once

#include "common/context.hpp"
#include "common/path.hpp"
#include "common/uuid.hpp"
#include "graph_editor.hpp"
#include "tool_context.hpp"

/**
 * Standalone authoring tool for `.bt_definition.xml` behavior tree assets.
 */
class BehaviorTreeEditorContext : public ToolContext {
public:
    static void Init();
    static void Destroy();
    static BehaviorTreeEditorContext& GetInst();

    void Initialize(int argc, char** argv) override;
    void Shutdown() override;

protected:
    void update() override;

private:
    void parseCmdArgs(int argc, char** argv);
    void showMainMenu();
    void showSidePanel();
    void newTree();
    void openFile();
    void save();
    void saveAs();
    void load(const Path& filename);
    void updateTitle();

    BehaviorTreeDefinition m_tree;
    BehaviorTreeGraphEditor m_graph;
    Path m_path;
    UUIDv4 m_uuid;
    bool m_dirty = false;
};

#define BT_EDITOR_CONTEXT ::BehaviorTreeEditorContext::GetInst()
