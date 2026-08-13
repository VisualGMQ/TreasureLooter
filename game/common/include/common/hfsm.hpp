#pragma once

#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/script/luabridge_include.hpp"
#include "common/script/script.hpp"

#include "schema/hfsm.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using HFSMNodeID = uint32_t;

class IHFSMBlackBoard;

class HFSMNode {
public:
    explicit HFSMNode(HFSMNodeID);
    virtual ~HFSMNode() = default;

    void AttachTo(HFSMNode& parent);
    void DetachFromParent();

    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual void OnUpdate() = 0;

    [[nodiscard]] HFSMNodeID GetID() const;

    [[nodiscard]] HFSMNode* GetParent() const { return m_parent; }

private:
    HFSMNodeID m_id{};
    HFSMNode* m_parent{};
    std::vector<HFSMNode*> m_children;
};

class LuauHFSMNode : public HFSMNode {
public:
    LuauHFSMNode(HFSMNodeID id, ScriptBinaryDataHandle handle);

    void OnEnter() override;
    void OnUpdate() override;
    void OnExit() override;

private:
    Script m_script;
};

class IHFSMBlackBoard {
public:
    virtual ~IHFSMBlackBoard() = default;
};

class LuauHFSMBlackBoard : public IHFSMBlackBoard {
public:
    LuauHFSMBlackBoard(lua_State* L);

    const luabridge::LuaRef& GetTable() const { return m_table; }

private:
    luabridge::LuaRef m_table;
};

class HFSMComponent {
public:
    explicit HFSMComponent(std::unique_ptr<IHFSMBlackBoard>&& blackboard) {
        m_blackboard = std::move(blackboard);
    }

    template <typename T, typename... Args>
    T* AddNode(HFSMNodeID id, Args&&... args) {
        auto result = m_nodes.emplace(
            id, std::make_unique<T>(id, std::forward<Args>(args)...));
        return static_cast<T*>(result.first->second.get());
    }

    void RemoveNode(HFSMNodeID id) { m_pending_remove_nodes.push_back(id); }

    void ChangeState(HFSMNodeID id) { m_pending_change_node = id; }

    HFSMNode* GetNode(HFSMNodeID id) {
        auto it = m_nodes.find(id);
        return it != m_nodes.end() ? it->second.get() : nullptr;
    }

    void Update();

    IHFSMBlackBoard& GetBlackBoard() const { return *m_blackboard; }

private:
    void doChangeState(HFSMNodeID id);

    std::unique_ptr<IHFSMBlackBoard> m_blackboard;
    std::unordered_map<HFSMNodeID, std::unique_ptr<HFSMNode>> m_nodes;

    HFSMNode* m_current{};
    std::optional<uint32_t> m_pending_change_node;
    std::vector<HFSMNodeID> m_pending_remove_nodes;
};

class LuauHFSMComponent : public HFSMComponent {
public:
    explicit LuauHFSMComponent(
        std::unique_ptr<LuauHFSMBlackBoard>&& blackboard)
        : HFSMComponent(std::move(blackboard)) {}

    luabridge::LuaRef GetBlackBoard() {
        return static_cast<LuauHFSMBlackBoard*>(
                   const_cast<IHFSMBlackBoard*>(
                       &HFSMComponent::GetBlackBoard()))
            ->GetTable();
    }
};

class HFSMComponentManager : public ComponentManager<HFSMComponent> {
public:
    LuauHFSMComponent* Create(Entity entity,
                              ScriptHFSMDefinitionHandle definition);

    LuauHFSMComponent* Get(Entity entity) {
        return static_cast<LuauHFSMComponent*>(
            ComponentManager<HFSMComponent>::Get(entity));
    }

    void Update();
};