#pragma once

#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/script/luabridge_include.hpp"
#include "common/script/script.hpp"

#include "schema/hfsm.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using HFSMNodeID = uint32_t;

class IHFSMBlackBoard;

class HFSMNode {
public:
    HFSMNode(HFSMNodeID id, std::string name);
    virtual ~HFSMNode() = default;

    void AttachTo(HFSMNode& parent);
    void DetachFromParent();

    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual void OnUpdate() = 0;

    [[nodiscard]] HFSMNodeID GetID() const;
    [[nodiscard]] const std::string& GetName() const;

    [[nodiscard]] HFSMNode* GetParent() const { return m_parent; }

    [[nodiscard]] const std::vector<HFSMNode*>& GetChildren() const {
        return m_children;
    }

private:
    HFSMNodeID m_id{};
    std::string m_name;
    HFSMNode* m_parent{};
    std::vector<HFSMNode*> m_children;
};

class LuaHFSMNode : public HFSMNode {
public:
    LuaHFSMNode(HFSMNodeID id, std::string name, LogicEntity entity,
                 ScriptBinaryDataHandle handle);

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

class LuaHFSMBlackBoard : public IHFSMBlackBoard {
public:
    LuaHFSMBlackBoard(lua_State* L);

    const luabridge::LuaRef& GetTable() const { return m_table; }

private:
    luabridge::LuaRef m_table;
};

class HFSMComponent {
public:
    explicit HFSMComponent(std::unique_ptr<IHFSMBlackBoard>&& blackboard);

    template <typename T, typename... Args>
    T* AddNode(HFSMNodeID id, Args&&... args) {
        auto result = m_nodes.emplace(
            id, std::make_unique<T>(id, std::forward<Args>(args)...));
        return static_cast<T*>(result.first->second.get());
    }

    void RemoveNode(HFSMNodeID id);

    void ChangeState(HFSMNodeID id);

    [[nodiscard]] HFSMNode* GetNode(HFSMNodeID id);

    void Update();

    [[nodiscard]] IHFSMBlackBoard& GetBlackBoard() const;

    void SetAssetName(std::string name) { m_asset_name = std::move(name); }

    [[nodiscard]] const std::string& GetAssetName() const {
        return m_asset_name;
    }

    [[nodiscard]] const std::unordered_map<HFSMNodeID,
                                           std::unique_ptr<HFSMNode>>&
    GetNodes() const {
        return m_nodes;
    }

    [[nodiscard]] HFSMNode* GetCurrentNode() const { return m_current; }

private:
    void doChangeState(HFSMNodeID id);
    void rebuildUpdateChain();

    std::string m_asset_name;
    std::unique_ptr<IHFSMBlackBoard> m_blackboard;
    std::unordered_map<HFSMNodeID, std::unique_ptr<HFSMNode>> m_nodes;

    HFSMNode* m_current{};
    std::vector<HFSMNode*> m_update_chain;
    std::optional<uint32_t> m_pending_change_node;
    std::vector<HFSMNodeID> m_pending_remove_nodes;
};

class LuaHFSMComponent : public HFSMComponent {
public:
    explicit LuaHFSMComponent(
        std::unique_ptr<LuaHFSMBlackBoard>&& blackboard);

    [[nodiscard]] luabridge::LuaRef GetBlackBoard() const;
};

class HFSMComponentManager : public ComponentManager<HFSMComponent> {
public:
    LuaHFSMComponent* Create(LogicEntity entity,
                              ScriptHFSMDefinitionHandle definition);

    [[nodiscard]] LuaHFSMComponent* Get(LogicEntity entity) override;

    void Update();
};