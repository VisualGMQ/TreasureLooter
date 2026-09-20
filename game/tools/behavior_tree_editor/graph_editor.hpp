#pragma once

#include "schema/behavior_tree.hpp"

#include <memory>

struct BehaviorTreeGraphEditorImpl;

/**
 * ImNodeFlow-based graph editor for a `BehaviorTreeDefinition` asset.
 *
 * The asset is the single source of truth; the node graph is a view that is
 * rebuilt from it whenever the structure changes. Node positions are kept in
 * memory only (auto-layout on open), so they are not serialized.
 */
class BehaviorTreeGraphEditor {
public:
    BehaviorTreeGraphEditor();
    ~BehaviorTreeGraphEditor();

    BehaviorTreeGraphEditor(const BehaviorTreeGraphEditor&) = delete;
    BehaviorTreeGraphEditor& operator=(const BehaviorTreeGraphEditor&) = delete;

    /**
     * Draws and edits the graph. Must be called inside an ImGui frame.
     * Sets `dirty` to true when the tree was modified.
     */
    void Update(BehaviorTreeDefinition& tree, bool& dirty);

    /** Forces the graph to be rebuilt from the tree on next Update. */
    void RequestRebuild();

    /** Clears stored positions so the next rebuild auto-lays-out the tree. */
    void AutoLayout();

    /**
     * Destroys the underlying ImNodeFlow editor (and its embedded ImGui
     * context). Must be called before the main ImGui context is shut down.
     */
    void Reset();

private:
    std::unique_ptr<BehaviorTreeGraphEditorImpl> m_impl;
};
