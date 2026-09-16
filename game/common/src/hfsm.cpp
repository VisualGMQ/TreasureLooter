#include "common/hfsm.hpp"

#include "common/asset_manager.hpp"
#include "common/context.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/profile.hpp"

#include "schema/serialize/hfsm.hpp"

#include <algorithm>
#include <string>
#include <string_view>

HFSMNode::HFSMNode(HFSMNodeID id, std::string name)
    : m_id{id}, m_name{std::move(name)} {}

void HFSMNode::AttachTo(HFSMNode& parent) {
    m_parent = &parent;
    parent.m_children.push_back(this);
}

void HFSMNode::DetachFromParent() {
    TL_RETURN_IF_NULL(m_parent);
    m_parent->m_children.erase(std::remove(m_parent->m_children.begin(),
                                           m_parent->m_children.end(), this),
                               m_parent->m_children.end());
    m_parent = nullptr;
}

HFSMNodeID HFSMNode::GetID() const {
    return m_id;
}

const std::string& HFSMNode::GetName() const {
    return m_name;
}

LuaHFSMNode::LuaHFSMNode(HFSMNodeID id, std::string name, LogicEntity entity,
                           ScriptBinaryDataHandle handle)
    : HFSMNode(id, std::move(name)), m_script(entity, handle) {}

void LuaHFSMNode::OnEnter() {
    m_script.callMethodNoArg("OnInit");
}

void LuaHFSMNode::OnUpdate() {
    m_script.callMethodNoArg("OnUpdate");
}

void LuaHFSMNode::OnExit() {
    m_script.callMethodNoArg("OnQuit");
}

LuaHFSMBlackBoard::LuaHFSMBlackBoard(lua_State* L)
    : m_table(luabridge::newTable(L)) {}

HFSMComponent::HFSMComponent(std::unique_ptr<IHFSMBlackBoard>&& blackboard) {
    m_blackboard = std::move(blackboard);
}

void HFSMComponent::RemoveNode(HFSMNodeID id) {
    m_pending_remove_nodes.push_back(id);
}

void HFSMComponent::ChangeState(HFSMNodeID id) {
    m_pending_change_node = id;
}

HFSMNode* HFSMComponent::GetNode(HFSMNodeID id) {
    auto it = m_nodes.find(id);
    return it != m_nodes.end() ? it->second.get() : nullptr;
}

void HFSMComponent::Update() {
    if (m_pending_change_node) {
        // Consume the pending request before applying it, so a ChangeState
        // issued from within OnInit/OnEnter (e.g. selecting the initial child
        // state) is not immediately discarded.
        HFSMNodeID next = *m_pending_change_node;
        m_pending_change_node.reset();
        doChangeState(next);
    }

    bool any_removed = false;
    for (HFSMNodeID id : m_pending_remove_nodes) {
        auto it = m_nodes.find(id);
        TL_CONTINUE_IF_FALSE(it != m_nodes.end());
        auto& node = it->second;
        node->OnExit();
        node->DetachFromParent();
        if (node.get() == m_current) {
            m_current = nullptr;
        }
        m_nodes.erase(it);
        any_removed = true;
    }
    m_pending_remove_nodes.clear();

    if (any_removed) {
        m_update_chain.clear();
    }

    for (HFSMNode* node : m_update_chain) {
        node->OnUpdate();
    }
}

IHFSMBlackBoard& HFSMComponent::GetBlackBoard() const {
    return *m_blackboard;
}

void HFSMComponent::doChangeState(HFSMNodeID id) {
    auto it = m_nodes.find(id);
    TL_RETURN_IF_FALSE_WITH_LOG(it != m_nodes.end(), LOGE,
                                "[HFSM]: unknown state id {}", id);
    HFSMNode* next = it->second.get();

    std::vector<HFSMNode*> exit_path;
    std::vector<HFSMNode*> enter_path;

    if (m_current) {
        std::unordered_set<HFSMNode*> current_ancestors;
        for (HFSMNode* node = m_current; node; node = node->GetParent()) {
            current_ancestors.insert(node);
        }

        HFSMNode* dca = nullptr;
        for (HFSMNode* node = next; node; node = node->GetParent()) {
            if (current_ancestors.find(node) != current_ancestors.end()) {
                dca = node;
                break;
            }
        }

        for (HFSMNode* node = m_current; node && node != dca;
             node = node->GetParent()) {
            exit_path.push_back(node);
        }

        for (HFSMNode* node = next; node && node != dca;
             node = node->GetParent()) {
            enter_path.push_back(node);
        }
    } else {
        for (HFSMNode* node = next; node; node = node->GetParent()) {
            enter_path.push_back(node);
        }
    }

    for (HFSMNode* node : exit_path) {
        node->OnExit();
    }

    std::reverse(enter_path.begin(), enter_path.end());
    for (HFSMNode* node : enter_path) {
        node->OnEnter();
    }

    m_current = next;
    rebuildUpdateChain();
}

void HFSMComponent::rebuildUpdateChain() {
    m_update_chain.clear();
    for (HFSMNode* node = m_current; node; node = node->GetParent()) {
        m_update_chain.push_back(node);
    }
    std::reverse(m_update_chain.begin(), m_update_chain.end());
}

LuaHFSMComponent::LuaHFSMComponent(
    std::unique_ptr<LuaHFSMBlackBoard>&& blackboard)
    : HFSMComponent(std::move(blackboard)) {}

luabridge::LuaRef LuaHFSMComponent::GetBlackBoard() const {
    return static_cast<LuaHFSMBlackBoard*>(
               const_cast<IHFSMBlackBoard*>(&HFSMComponent::GetBlackBoard()))
        ->GetTable();
}

LuaHFSMComponent* HFSMComponentManager::Create(
    LogicEntity entity, ScriptHFSMDefinitionHandle definition) {
    TL_RETURN_VALUE_IF_NULL_WITH_LOG(definition.Get(), nullptr, LOGE,
                                     "[HFSM]: definition is null");

    const auto& nodes = definition->m_nodes;
    TL_RETURN_VALUE_IF_FALSE_WITH_LOG(!nodes.empty(), nullptr, LOGE,
                                      "[HFSM]: definition has no nodes");

    auto& script_manager =
        COMMON_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>();
    lua_State* L = script_manager.GetUnderlyingVM();
    TL_RETURN_VALUE_IF_NULL_WITH_LOG(L, nullptr, LOGE, "[HFSM]: VM is null");

    RegisterEntityByDerive<LuaHFSMComponent>(
        entity, std::make_unique<LuaHFSMBlackBoard>(L));
    LuaHFSMComponent* component = Get(entity);
    TL_RETURN_VALUE_IF_NULL_WITH_LOG(component, nullptr, LOGE,
                                     "[HFSM]: register entity {} failed",
                                     entity);

    if (const Path* filename = definition.GetFilename()) {
        component->SetAssetName(filename->stem().string());
    }

    for (uint32_t i = 0; i < nodes.size(); i++) {
        auto script_handle = script_manager.Load(nodes[i].m_script);
        if (!script_handle) {
            LOGE("[HFSM]: load node {} script {} failed", i,
                 nodes[i].m_script);
            continue;
        }
        component->AddNode<LuaHFSMNode>(i, nodes[i].m_name, entity,
                                         script_handle);
    }

    for (uint32_t i = 0; i < nodes.size(); i++) {
        HFSMNode* parent = component->GetNode(i);
        TL_CONTINUE_IF_NULL(parent);
        for (uint32_t child_id : nodes[i].m_children) {
            if (child_id >= nodes.size()) {
                LOGE("[HFSM]: node {} child id {} out of range", i, child_id);
                continue;
            }
            HFSMNode* child = component->GetNode(child_id);
            TL_CONTINUE_IF_NULL(child);
            child->AttachTo(*parent);
        }
    }

    std::unordered_set<uint32_t> child_ids;
    for (const auto& node : nodes) {
        child_ids.insert(node.m_children.begin(), node.m_children.end());
    }
    uint32_t root_id = 0;
    for (uint32_t i = 0; i < nodes.size(); i++) {
        if (child_ids.find(i) == child_ids.end()) {
            root_id = i;
            break;
        }
    }
    component->ChangeState(root_id);

    return component;
}

LuaHFSMComponent* HFSMComponentManager::Get(LogicEntity entity) {
    return static_cast<LuaHFSMComponent*>(
        ComponentManager<HFSMComponent>::Get(entity));
}

void HFSMComponentManager::Update() {
    PROFILE_SECTION();

    for (auto& [entity, component] : m_components) {
        TL_CONTINUE_IF_FALSE(component.m_enable);
        component.m_component->Update();
    }
}