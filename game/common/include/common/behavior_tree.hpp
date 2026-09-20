#pragma once

#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/script/luabridge_include.hpp"
#include "common/script/script.hpp"

#include "schema/behavior_tree.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using BehaviorTreeNodeID = uint32_t;

class BehaviorTreeBlackBoard {
public:
    virtual ~BehaviorTreeBlackBoard() = default;
};

class LuaBehaviorTreeBlackBoard : public BehaviorTreeBlackBoard {
public:
    explicit LuaBehaviorTreeBlackBoard(lua_State* L);

    [[nodiscard]] const luabridge::LuaRef& GetTable() const { return m_table; }

private:
    luabridge::LuaRef m_table;
};

class BehaviorTreeNode {
public:
    BehaviorTreeNode(BehaviorTreeNodeID id, std::string name,
                     BehaviorTreeNodeType type);
    virtual ~BehaviorTreeNode() = default;

    void AddChild(BehaviorTreeNode& child);

    BehaviorTreeStatus Update(BehaviorTreeBlackBoard& blackboard,
                              TimeType elapse);
    void Stop(BehaviorTreeBlackBoard& blackboard);

    [[nodiscard]] BehaviorTreeNodeID GetID() const { return m_id; }

    [[nodiscard]] const std::string& GetName() const { return m_name; }

    [[nodiscard]] BehaviorTreeNodeType GetType() const { return m_type; }

    [[nodiscard]] bool IsRunning() const { return m_running; }

    [[nodiscard]] const std::vector<BehaviorTreeNode*>& GetChildren() const {
        return m_children;
    }

protected:
    virtual void onEnter(BehaviorTreeBlackBoard& /*blackboard*/) {}

    virtual void onExit(BehaviorTreeBlackBoard& /*blackboard*/) {}

    virtual BehaviorTreeStatus onUpdate(BehaviorTreeBlackBoard& blackboard,
                                        TimeType elapse) = 0;

    BehaviorTreeNodeID m_id{};
    std::string m_name;
    BehaviorTreeNodeType m_type{};
    std::vector<BehaviorTreeNode*> m_children;
    bool m_running = false;
};

/** Runs children in order until one fails; fails on the first failure. */
class SequenceNode : public BehaviorTreeNode {
public:
    using BehaviorTreeNode::BehaviorTreeNode;

protected:
    void onEnter(BehaviorTreeBlackBoard& blackboard) override;
    BehaviorTreeStatus onUpdate(BehaviorTreeBlackBoard& blackboard,
                                TimeType elapse) override;

private:
    size_t m_index = 0;
};

/** Runs children in order until one succeeds; fails when all fail. */
class SelectorNode : public BehaviorTreeNode {
public:
    using BehaviorTreeNode::BehaviorTreeNode;

protected:
    void onEnter(BehaviorTreeBlackBoard& blackboard) override;
    BehaviorTreeStatus onUpdate(BehaviorTreeBlackBoard& blackboard,
                                TimeType elapse) override;

private:
    size_t m_index = 0;
};

/** Ticks every child each frame and aggregates the result by policy. */
class ParallelNode : public BehaviorTreeNode {
public:
    ParallelNode(BehaviorTreeNodeID id, std::string name,
                 BehaviorTreeNodeType type,
                 ParallelSuccessPolicy success_policy,
                 ParallelFailurePolicy failure_policy);

protected:
    void onEnter(BehaviorTreeBlackBoard& blackboard) override;
    BehaviorTreeStatus onUpdate(BehaviorTreeBlackBoard& blackboard,
                                TimeType elapse) override;

private:
    ParallelSuccessPolicy m_success_policy;
    ParallelFailurePolicy m_failure_policy;
    std::vector<char> m_resolved;
    int m_success_count = 0;
    int m_failure_count = 0;
};

/** Inverts Success/Failure of its single child. */
class InverterNode : public BehaviorTreeNode {
public:
    using BehaviorTreeNode::BehaviorTreeNode;

protected:
    BehaviorTreeStatus onUpdate(BehaviorTreeBlackBoard& blackboard,
                                TimeType elapse) override;
};

/** Always returns Success for its single child (Running passes through). */
class SucceederNode : public BehaviorTreeNode {
public:
    using BehaviorTreeNode::BehaviorTreeNode;

protected:
    BehaviorTreeStatus onUpdate(BehaviorTreeBlackBoard& blackboard,
                                TimeType elapse) override;
};

/** Always returns Failure for its single child (Running passes through). */
class FailerNode : public BehaviorTreeNode {
public:
    using BehaviorTreeNode::BehaviorTreeNode;

protected:
    BehaviorTreeStatus onUpdate(BehaviorTreeBlackBoard& blackboard,
                                TimeType elapse) override;
};

/** Repeats its single child `count` times (count <= 0 means forever). */
class RepeatNode : public BehaviorTreeNode {
public:
    RepeatNode(BehaviorTreeNodeID id, std::string name,
               BehaviorTreeNodeType type, int count);

protected:
    void onEnter(BehaviorTreeBlackBoard& blackboard) override;
    BehaviorTreeStatus onUpdate(BehaviorTreeBlackBoard& blackboard,
                                TimeType elapse) override;

private:
    int m_count = 0;
    int m_done = 0;
};

/** Repeats its child until it fails, then returns Success. */
class UntilFailNode : public BehaviorTreeNode {
public:
    using BehaviorTreeNode::BehaviorTreeNode;

protected:
    BehaviorTreeStatus onUpdate(BehaviorTreeBlackBoard& blackboard,
                                TimeType elapse) override;
};

/** Waits `time` seconds, then returns Success. */
class WaitNode : public BehaviorTreeNode {
public:
    WaitNode(BehaviorTreeNodeID id, std::string name, BehaviorTreeNodeType type,
             float time);

protected:
    void onEnter(BehaviorTreeBlackBoard& blackboard) override;
    BehaviorTreeStatus onUpdate(BehaviorTreeBlackBoard& blackboard,
                                TimeType elapse) override;

private:
    TimeType m_time = 0;
    TimeType m_elapsed = 0;
};

class LuaBehaviorTreeNode : public BehaviorTreeNode {
public:
    LuaBehaviorTreeNode(BehaviorTreeNodeID id, std::string name,
                        BehaviorTreeNodeType type, LogicEntity entity,
                        ScriptBinaryDataHandle handle);

protected:
    void onEnter(BehaviorTreeBlackBoard& blackboard) override;
    void onExit(BehaviorTreeBlackBoard& blackboard) override;
    BehaviorTreeStatus onUpdate(BehaviorTreeBlackBoard& blackboard,
                                TimeType elapse) override;

private:
    Script m_script;
};

class BehaviorTreeComponent {
public:
    explicit BehaviorTreeComponent(
        std::unique_ptr<BehaviorTreeBlackBoard>&& blackboard);

    template <typename T, typename... Args>
    T* AddNode(BehaviorTreeNodeID id, Args&&... args) {
        auto result = m_nodes.emplace(
            id, std::make_unique<T>(id, std::forward<Args>(args)...));
        return static_cast<T*>(result.first->second.get());
    }

    [[nodiscard]] BehaviorTreeNode* GetNode(BehaviorTreeNodeID id) const;

    void SetRoot(BehaviorTreeNodeID id);

    [[nodiscard]] BehaviorTreeNode* GetRoot() const { return m_root; }

    void Update(TimeType elapse);

    [[nodiscard]] BehaviorTreeBlackBoard& GetBlackBoard() const;

    void SetAssetName(std::string name) { m_asset_name = std::move(name); }

    [[nodiscard]] const std::string& GetAssetName() const {
        return m_asset_name;
    }

    [[nodiscard]] const std::unordered_map<BehaviorTreeNodeID,
                                           std::unique_ptr<BehaviorTreeNode>>&
    GetNodes() const {
        return m_nodes;
    }

private:
    std::string m_asset_name;
    std::unique_ptr<BehaviorTreeBlackBoard> m_blackboard;
    std::unordered_map<BehaviorTreeNodeID, std::unique_ptr<BehaviorTreeNode>>
        m_nodes;
    BehaviorTreeNode* m_root{};
};

class LuaBehaviorTreeComponent : public BehaviorTreeComponent {
public:
    explicit LuaBehaviorTreeComponent(
        std::unique_ptr<LuaBehaviorTreeBlackBoard>&& blackboard);

    [[nodiscard]] luabridge::LuaRef GetBlackBoard() const;
};

class BehaviorTreeComponentManager
    : public ComponentManager<BehaviorTreeComponent> {
public:
    LuaBehaviorTreeComponent* Create(LogicEntity entity,
                                     BehaviorTreeDefinitionHandle definition);

    [[nodiscard]] LuaBehaviorTreeComponent* Get(LogicEntity entity) override;

    void Update(TimeType elapse);
};
