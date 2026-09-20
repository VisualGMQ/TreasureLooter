#include "common/behavior_tree.hpp"

#include "common/asset_manager.hpp"
#include "common/context.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/profile.hpp"

#include "schema/serialize/behavior_tree.hpp"

#include <algorithm>
#include <string>
#include <string_view>

// -----------------------------------------------------------------------------
// Blackboard
// -----------------------------------------------------------------------------

LuaBehaviorTreeBlackBoard::LuaBehaviorTreeBlackBoard(lua_State* L)
    : m_table(luabridge::newTable(L)) {}

// -----------------------------------------------------------------------------
// BehaviorTreeNode
// -----------------------------------------------------------------------------

BehaviorTreeNode::BehaviorTreeNode(BehaviorTreeNodeID id, std::string name,
                                   BehaviorTreeNodeType type)
    : m_id{id}, m_name{std::move(name)}, m_type{type} {}

void BehaviorTreeNode::AddChild(BehaviorTreeNode& child) {
    m_children.push_back(&child);
}

BehaviorTreeStatus BehaviorTreeNode::Update(BehaviorTreeBlackBoard& blackboard,
                                          TimeType elapse) {
    if (!m_running) {
        m_running = true;
        onEnter(blackboard);
    }

    const BehaviorTreeStatus status = onUpdate(blackboard, elapse);
    if (status != BehaviorTreeStatus::Running) {
        Stop(blackboard);
    }
    return status;
}

void BehaviorTreeNode::Stop(BehaviorTreeBlackBoard& blackboard) {
    if (!m_running) {
        return;
    }
    onExit(blackboard);
    for (BehaviorTreeNode* child : m_children) {
        child->Stop(blackboard);
    }
    m_running = false;
}

// -----------------------------------------------------------------------------
// SequenceNode
// -----------------------------------------------------------------------------

void SequenceNode::onEnter(BehaviorTreeBlackBoard& /*blackboard*/) {
    m_index = 0;
}

BehaviorTreeStatus SequenceNode::onUpdate(BehaviorTreeBlackBoard& blackboard,
                                        TimeType elapse) {
    while (m_index < m_children.size()) {
        const BehaviorTreeStatus status =
            m_children[m_index]->Update(blackboard, elapse);
        if (status == BehaviorTreeStatus::Failure) {
            return BehaviorTreeStatus::Failure;
        }
        if (status == BehaviorTreeStatus::Running) {
            return BehaviorTreeStatus::Running;
        }
        ++m_index;
    }
    return BehaviorTreeStatus::Success;
}

// -----------------------------------------------------------------------------
// SelectorNode
// -----------------------------------------------------------------------------

void SelectorNode::onEnter(BehaviorTreeBlackBoard& /*blackboard*/) {
    m_index = 0;
}

BehaviorTreeStatus SelectorNode::onUpdate(BehaviorTreeBlackBoard& blackboard,
                                        TimeType elapse) {
    while (m_index < m_children.size()) {
        const BehaviorTreeStatus status =
            m_children[m_index]->Update(blackboard, elapse);
        if (status == BehaviorTreeStatus::Success) {
            return BehaviorTreeStatus::Success;
        }
        if (status == BehaviorTreeStatus::Running) {
            return BehaviorTreeStatus::Running;
        }
        ++m_index;
    }
    return BehaviorTreeStatus::Failure;
}

// -----------------------------------------------------------------------------
// ParallelNode
// -----------------------------------------------------------------------------

ParallelNode::ParallelNode(BehaviorTreeNodeID id, std::string name,
                           BehaviorTreeNodeType type,
                           ParallelSuccessPolicy success_policy,
                           ParallelFailurePolicy failure_policy)
    : BehaviorTreeNode(id, std::move(name), type),
      m_success_policy(success_policy),
      m_failure_policy(failure_policy) {}

void ParallelNode::onEnter(BehaviorTreeBlackBoard& /*blackboard*/) {
    m_resolved.assign(m_children.size(), 0);
    m_success_count = 0;
    m_failure_count = 0;
}

BehaviorTreeStatus ParallelNode::onUpdate(BehaviorTreeBlackBoard& blackboard,
                                        TimeType elapse) {
    const size_t count = m_children.size();
    if (m_resolved.size() != count) {
        m_resolved.assign(count, 0);
        m_success_count = 0;
        m_failure_count = 0;
    }

    for (size_t i = 0; i < count; ++i) {
        if (m_resolved[i]) {
            continue;
        }
        const BehaviorTreeStatus status =
            m_children[i]->Update(blackboard, elapse);
        if (status == BehaviorTreeStatus::Success) {
            m_resolved[i] = 1;
            ++m_success_count;
        } else if (status == BehaviorTreeStatus::Failure) {
            m_resolved[i] = 1;
            ++m_failure_count;
        }
    }

    const bool success_reached =
        (m_success_policy == ParallelSuccessPolicy::All)
            ? (m_success_count == static_cast<int>(count))
            : (m_success_count >= 1);
    const bool failure_reached =
        (m_failure_policy == ParallelFailurePolicy::Any)
            ? (m_failure_count >= 1)
            : (m_failure_count == static_cast<int>(count));

    // Failure wins when both conditions are reached on the same frame.
    if (failure_reached) {
        return BehaviorTreeStatus::Failure;
    }
    if (success_reached) {
        return BehaviorTreeStatus::Success;
    }
    return BehaviorTreeStatus::Running;
}

// -----------------------------------------------------------------------------
// Inverter / Succeeder / Failer
// -----------------------------------------------------------------------------

BehaviorTreeStatus InverterNode::onUpdate(BehaviorTreeBlackBoard& blackboard,
                                        TimeType elapse) {
    if (m_children.empty()) {
        return BehaviorTreeStatus::Failure;
    }
    const BehaviorTreeStatus status = m_children[0]->Update(blackboard, elapse);
    if (status == BehaviorTreeStatus::Success) {
        return BehaviorTreeStatus::Failure;
    }
    if (status == BehaviorTreeStatus::Failure) {
        return BehaviorTreeStatus::Success;
    }
    return BehaviorTreeStatus::Running;
}

BehaviorTreeStatus SucceederNode::onUpdate(BehaviorTreeBlackBoard& blackboard,
                                         TimeType elapse) {
    if (m_children.empty()) {
        return BehaviorTreeStatus::Success;
    }
    const BehaviorTreeStatus status = m_children[0]->Update(blackboard, elapse);
    if (status == BehaviorTreeStatus::Running) {
        return BehaviorTreeStatus::Running;
    }
    return BehaviorTreeStatus::Success;
}

BehaviorTreeStatus FailerNode::onUpdate(BehaviorTreeBlackBoard& blackboard,
                                      TimeType elapse) {
    if (m_children.empty()) {
        return BehaviorTreeStatus::Failure;
    }
    const BehaviorTreeStatus status = m_children[0]->Update(blackboard, elapse);
    if (status == BehaviorTreeStatus::Running) {
        return BehaviorTreeStatus::Running;
    }
    return BehaviorTreeStatus::Failure;
}

// -----------------------------------------------------------------------------
// RepeatNode / UntilFailNode / WaitNode
// -----------------------------------------------------------------------------

RepeatNode::RepeatNode(BehaviorTreeNodeID id, std::string name,
                       BehaviorTreeNodeType type, int count)
    : BehaviorTreeNode(id, std::move(name), type), m_count(count) {}

void RepeatNode::onEnter(BehaviorTreeBlackBoard& /*blackboard*/) {
    m_done = 0;
}

BehaviorTreeStatus RepeatNode::onUpdate(BehaviorTreeBlackBoard& blackboard,
                                      TimeType elapse) {
    if (m_children.empty()) {
        return BehaviorTreeStatus::Failure;
    }
    const BehaviorTreeStatus status = m_children[0]->Update(blackboard, elapse);
    if (status == BehaviorTreeStatus::Running) {
        return BehaviorTreeStatus::Running;
    }
    if (status == BehaviorTreeStatus::Failure) {
        return BehaviorTreeStatus::Failure;
    }

    ++m_done;
    if (m_count > 0 && m_done >= m_count) {
        return BehaviorTreeStatus::Success;
    }
    // The child ended, so it will re-enter on the next tick.
    return BehaviorTreeStatus::Running;
}

BehaviorTreeStatus UntilFailNode::onUpdate(BehaviorTreeBlackBoard& blackboard,
                                         TimeType elapse) {
    if (m_children.empty()) {
        return BehaviorTreeStatus::Success;
    }
    const BehaviorTreeStatus status = m_children[0]->Update(blackboard, elapse);
    if (status == BehaviorTreeStatus::Failure) {
        return BehaviorTreeStatus::Success;
    }
    // Success repeats (child re-enters next tick); Running continues.
    return BehaviorTreeStatus::Running;
}

WaitNode::WaitNode(BehaviorTreeNodeID id, std::string name,
                   BehaviorTreeNodeType type, float time)
    : BehaviorTreeNode(id, std::move(name), type),
      m_time(static_cast<TimeType>(time)) {}

void WaitNode::onEnter(BehaviorTreeBlackBoard& /*blackboard*/) {
    m_elapsed = 0;
}

BehaviorTreeStatus WaitNode::onUpdate(BehaviorTreeBlackBoard& /*blackboard*/,
                                    TimeType elapse) {
    m_elapsed += elapse;
    if (m_elapsed >= m_time) {
        return BehaviorTreeStatus::Success;
    }
    return BehaviorTreeStatus::Running;
}

// -----------------------------------------------------------------------------
// LuaBehaviorTreeNode
// -----------------------------------------------------------------------------

LuaBehaviorTreeNode::LuaBehaviorTreeNode(BehaviorTreeNodeID id,
                                         std::string name,
                                         BehaviorTreeNodeType type,
                                         LogicEntity entity,
                                         ScriptBinaryDataHandle handle)
    : BehaviorTreeNode(id, std::move(name), type),
      m_script(entity, handle) {}

void LuaBehaviorTreeNode::onEnter(BehaviorTreeBlackBoard& /*blackboard*/) {
    m_script.callMethodNoArg("OnInit");
}

void LuaBehaviorTreeNode::onExit(BehaviorTreeBlackBoard& /*blackboard*/) {
    m_script.callMethodNoArg("OnQuit");
}

BehaviorTreeStatus LuaBehaviorTreeNode::onUpdate(
    BehaviorTreeBlackBoard& blackboard, TimeType /*elapse*/) {
    const auto* lua_blackboard =
        dynamic_cast<const LuaBehaviorTreeBlackBoard*>(&blackboard);
    if (!lua_blackboard) {
        return BehaviorTreeStatus::Failure;
    }

    const std::optional<int> result =
        m_script.callMethodReturningInt("OnUpdate", lua_blackboard->GetTable());
    if (!result) {
        return BehaviorTreeStatus::Failure;
    }

    switch (static_cast<BehaviorTreeStatus>(*result)) {
        case BehaviorTreeStatus::Success:
            return BehaviorTreeStatus::Success;
        case BehaviorTreeStatus::Failure:
            return BehaviorTreeStatus::Failure;
        case BehaviorTreeStatus::Running:
            return BehaviorTreeStatus::Running;
    }
    return BehaviorTreeStatus::Failure;
}

// -----------------------------------------------------------------------------
// BehaviorTreeComponent
// -----------------------------------------------------------------------------

BehaviorTreeComponent::BehaviorTreeComponent(
    std::unique_ptr<BehaviorTreeBlackBoard>&& blackboard)
    : m_blackboard(std::move(blackboard)) {}

BehaviorTreeNode* BehaviorTreeComponent::GetNode(BehaviorTreeNodeID id) const {
    if (auto it = m_nodes.find(id); it != m_nodes.end()) {
        return it->second.get();
    }
    return nullptr;
}

void BehaviorTreeComponent::SetRoot(BehaviorTreeNodeID id) {
    m_root = GetNode(id);
}

void BehaviorTreeComponent::Update(TimeType elapse) {
    if (!m_root || !m_blackboard) {
        return;
    }
    m_root->Update(*m_blackboard, elapse);
}

BehaviorTreeBlackBoard& BehaviorTreeComponent::GetBlackBoard() const {
    return *m_blackboard;
}

LuaBehaviorTreeComponent::LuaBehaviorTreeComponent(
    std::unique_ptr<LuaBehaviorTreeBlackBoard>&& blackboard)
    : BehaviorTreeComponent(std::move(blackboard)) {}

luabridge::LuaRef LuaBehaviorTreeComponent::GetBlackBoard() const {
    const auto* blackboard = static_cast<const LuaBehaviorTreeBlackBoard*>(
        &BehaviorTreeComponent::GetBlackBoard());
    return blackboard->GetTable();
}

// -----------------------------------------------------------------------------
// BehaviorTreeComponentManager
// -----------------------------------------------------------------------------

LuaBehaviorTreeComponent* BehaviorTreeComponentManager::Create(
    LogicEntity entity, BehaviorTreeDefinitionHandle definition) {
    TL_RETURN_VALUE_IF_NULL_WITH_LOG(definition.Get(), nullptr, LOGE,
                                     "[BT]: definition is null");

    const auto& nodes = definition->m_nodes;
    TL_RETURN_VALUE_IF_FALSE_WITH_LOG(!nodes.empty(), nullptr, LOGE,
                                      "[BT]: definition has no nodes");

    auto& script_manager =
        COMMON_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>();
    lua_State* L = script_manager.GetUnderlyingVM();
    TL_RETURN_VALUE_IF_NULL_WITH_LOG(L, nullptr, LOGE, "[BT]: VM is null");

    RegisterEntityByDerive<LuaBehaviorTreeComponent>(
        entity, std::make_unique<LuaBehaviorTreeBlackBoard>(L));
    LuaBehaviorTreeComponent* component = Get(entity);
    TL_RETURN_VALUE_IF_NULL_WITH_LOG(component, nullptr, LOGE,
                                     "[BT]: register entity {} failed", entity);

    if (const Path* filename = definition.GetFilename()) {
        component->SetAssetName(filename->stem().string());
    }

    for (const auto& node : nodes) {
        const BehaviorTreeNodeID id = node.m_id;
        if (component->GetNode(id)) {
            LOGE("[BT]: duplicated node id {}", id);
            continue;
        }

        switch (node.m_type) {
            case BehaviorTreeNodeType::Sequence:
                component->AddNode<SequenceNode>(id, node.m_name, node.m_type);
                break;
            case BehaviorTreeNodeType::Selector:
                component->AddNode<SelectorNode>(id, node.m_name, node.m_type);
                break;
            case BehaviorTreeNodeType::Parallel:
                component->AddNode<ParallelNode>(
                    id, node.m_name, node.m_type, node.m_success_policy,
                    node.m_failure_policy);
                break;
            case BehaviorTreeNodeType::Inverter:
                component->AddNode<InverterNode>(id, node.m_name, node.m_type);
                break;
            case BehaviorTreeNodeType::Succeeder:
                component->AddNode<SucceederNode>(id, node.m_name, node.m_type);
                break;
            case BehaviorTreeNodeType::Failer:
                component->AddNode<FailerNode>(id, node.m_name, node.m_type);
                break;
            case BehaviorTreeNodeType::Repeat:
                component->AddNode<RepeatNode>(id, node.m_name, node.m_type,
                                               node.m_count);
                break;
            case BehaviorTreeNodeType::UntilFail:
                component->AddNode<UntilFailNode>(id, node.m_name, node.m_type);
                break;
            case BehaviorTreeNodeType::Wait:
                component->AddNode<WaitNode>(id, node.m_name, node.m_type,
                                             node.m_time);
                break;
            case BehaviorTreeNodeType::Action:
            case BehaviorTreeNodeType::Condition: {
                auto handle = script_manager.Load(node.m_script);
                if (!handle) {
                    LOGE("[BT]: load node {} script {} failed", id,
                         node.m_script);
                    break;
                }
                component->AddNode<LuaBehaviorTreeNode>(
                    id, node.m_name, node.m_type, entity, handle);
                break;
            }
        }
    }

    std::unordered_set<BehaviorTreeNodeID> child_ids;
    for (const auto& node : nodes) {
        for (BehaviorTreeNodeID child_id : node.m_children) {
            child_ids.insert(child_id);
            BehaviorTreeNode* parent = component->GetNode(node.m_id);
            BehaviorTreeNode* child = component->GetNode(child_id);
            if (!parent || !child) {
                LOGE("[BT]: link node {} -> {} failed", node.m_id, child_id);
                continue;
            }
            parent->AddChild(*child);
        }
    }

    BehaviorTreeNodeID root_id = 0;
    bool root_found = false;
    for (const auto& node : nodes) {
        if (child_ids.find(node.m_id) == child_ids.end()) {
            root_id = node.m_id;
            root_found = true;
            break;
        }
    }

    if (root_found) {
        component->SetRoot(root_id);
    } else {
        LOGE("[BT]: no root found (cycle?)");
    }

    return component;
}

LuaBehaviorTreeComponent* BehaviorTreeComponentManager::Get(LogicEntity entity) {
    return static_cast<LuaBehaviorTreeComponent*>(
        ComponentManager<BehaviorTreeComponent>::Get(entity));
}

void BehaviorTreeComponentManager::Update(TimeType elapse) {
    PROFILE_SECTION();

    for (auto& [entity, component] : m_components) {
        TL_CONTINUE_IF_FALSE(component.m_enable);
        component.m_component->Update(elapse);
    }
}
