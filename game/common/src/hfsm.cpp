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

HFSMNode::HFSMNode(HFSMNodeID id) : m_id{id} {}

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

LuauHFSMNode::LuauHFSMNode(HFSMNodeID id, ScriptBinaryDataHandle handle)
    : HFSMNode(id), m_script(null_entity, handle) {}

void LuauHFSMNode::OnEnter() {
    m_script.callMethodNoArg("OnInit");
}

void LuauHFSMNode::OnUpdate() {
    m_script.callMethodNoArg("OnUpdate");
}

void LuauHFSMNode::OnExit() {
    m_script.callMethodNoArg("OnQuit");
}

LuauHFSMBlackBoard::LuauHFSMBlackBoard(lua_State* L)
    : m_table(luabridge::newTable(L)) {}

void HFSMComponent::Update() {
    if (m_pending_change_node) {
        doChangeState(*m_pending_change_node);
        m_pending_change_node.reset();
    }

    for (HFSMNodeID id : m_pending_remove_nodes) {
        auto it = m_nodes.find(id);
        TL_CONTINUE_IF_FALSE(it != m_nodes.end());
        auto& node = it->second;
        node->OnExit();
        node->DetachFromParent();
        m_nodes.erase(it);
    }
    m_pending_remove_nodes.clear();

    if (m_current) {
        m_current->OnUpdate();
    }
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
}

LuauHFSMComponent* HFSMComponentManager::Create(
    Entity entity, ScriptHFSMDefinitionHandle definition) {
    TL_RETURN_VALUE_IF_NULL_WITH_LOG(definition.Get(), nullptr, LOGE,
                                     "[HFSM]: definition is null");

    const auto& nodes = definition->m_nodes;
    TL_RETURN_VALUE_IF_FALSE_WITH_LOG(!nodes.empty(), nullptr, LOGE,
                                      "[HFSM]: definition has no nodes");

    auto& script_manager =
        COMMON_CONTEXT.m_assets_manager->GetManager<ScriptBinaryData>();
    lua_State* L = script_manager.GetUnderlyingVM();
    TL_RETURN_VALUE_IF_NULL_WITH_LOG(L, nullptr, LOGE, "[HFSM]: VM is null");

    RegisterEntityByDerive<LuauHFSMComponent>(
        entity, std::make_unique<LuauHFSMBlackBoard>(L));
    LuauHFSMComponent* component = Get(entity);
    TL_RETURN_VALUE_IF_NULL_WITH_LOG(component, nullptr, LOGE,
                                     "[HFSM]: register entity {} failed",
                                     entity);

    for (uint32_t i = 0; i < nodes.size(); i++) {
        auto script_handle = script_manager.Load(nodes[i].m_script);
        if (!script_handle) {
            LOGE("[HFSM]: load node {} script {} failed", i,
                 nodes[i].m_script);
            continue;
        }
        component->AddNode<LuauHFSMNode>(i, script_handle);
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

void HFSMComponentManager::Update() {
    PROFILE_SECTION();

    for (auto& [entity, component] : m_components) {
        TL_CONTINUE_IF_FALSE(component.m_enable);
        component.m_component->Update();
    }
}