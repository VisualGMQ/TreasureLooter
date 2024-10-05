#include "go_hierarchy_watcher.hpp"
#include "context.hpp"
#include "macro.hpp"

namespace tl {

void GOHierarchyWatcher::Update() {
    auto& ctx = Context::GetInst();
    if (ImGui::Begin("hierarchy")) {
        ImGui::Checkbox("draw GO", &ctx.debugMgr->enableDrawGO);

        std::string curSceneName = "<no scene>";
        auto& sceneMgr = Context::GetInst().sceneMgr;
        for (auto& [name, scene] : sceneMgr->GetAllScenes()) {
            if (sceneMgr->GetCurScene() == &scene) {
                curSceneName = name;
                break;
            }
        }

        if (ImGui::BeginCombo("scene", curSceneName.c_str())) {
            for (auto& [name, scene] : sceneMgr->GetAllScenes()) {
                if (ImGui::Selectable(name.c_str(), curSceneName == name)) {
                    sceneMgr->ChangeScene(name);
                }
            }
            ImGui::EndCombo();
        }

        GameObject* root = ctx.sceneMgr->GetCurScene()->GetRootGO();
        if (!root) {
            ImGui::End();
            return;
        }
        int id = 0;
        updateRecursive( *root, id, true, false);
    }
    ImGui::End();

    applyGOMove();
    goMoveInfo_.Reset();
    if (shouldChangeDraggingState_) {
        draggingGOID_ = GameObjectID::Null;
        shouldChangeDraggingState_ = false;
    }
}

void GOHierarchyWatcher::updateRecursive(GameObject& go, int& id,
                                         bool isParentOpen, bool isParentDragging) {
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow |
                                   ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                   ImGuiTreeNodeFlags_SpanAvailWidth;
    bool hasChildren = !go.GetChildren().empty();
    if (!hasChildren) {
        nodeFlags |=
            ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }
    if (selectedGO_ && selectedGO_ == go.GetID()) {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    auto scene = Context::GetInst().sceneMgr->GetCurScene();
    bool isNodeOpen = false;

    std::string nameWithID = go.name + "###" + std::to_string(++id);

    auto goID = go.GetID();
    bool isDragging = false;

    if (isParentOpen) {
        isNodeOpen = ImGui::TreeNodeEx(nameWithID.c_str(), nodeFlags);
        if (ImGui::IsItemClicked()) {
            selectedGO_ = goID;
        }

        if (goID != scene->GetRootGOID()) {
            isDragging = ImGui::BeginDragDropSource();
        }
        if (isDragging) {
            draggingGOID_ = goID;
            ImGui::SetDragDropPayload("go", &goID, sizeof(goID));
            ImGui::Text("%s", go.name.c_str());
            ImGui::EndDragDropSource();
        } else {
            bool toBeChild = !ImGui::IsKeyDown(ImGuiKey_S);
            if (!isParentDragging &&
                (toBeChild || goID != scene->GetRootGOID()) &&
                ImGui::BeginDragDropTarget()) {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("go");
                if (payload && payload->Data) {
                    GameObjectID* id = (GameObjectID*)payload->Data;
                    
                    goMoveInfo_.source = *id;
                    goMoveInfo_.target = goID;
                    goMoveInfo_.toBeChild = toBeChild;
                }
                ImGui::EndDragDropTarget();
            }

            if (draggingGOID_ == goID) {
                shouldChangeDraggingState_ = true;
            }
        }
    }

    for (auto child : go.GetChildren()) {
        GameObject* go = Context::GetInst().goMgr->Find(child);
        if (!go) {
            continue;
        }
        updateRecursive(*go, id, isNodeOpen,
                        isParentDragging || draggingGOID_ == goID);
    }
    if (isNodeOpen && hasChildren) {
        ImGui::TreePop();
    }
}

void GOHierarchyWatcher::applyGOMove() {
    TL_RETURN_IF(goMoveInfo_);

    auto& goMgr = Context::GetInst().goMgr;
    auto scene = Context::GetInst().sceneMgr->GetCurScene();
    GameObject* source = goMgr->Find(goMoveInfo_.source);
    TL_RETURN_IF(source);

    GameObject* parent = goMgr->Find(source->GetParentID());
    TL_RETURN_IF(parent);

    GameObject* target = goMgr->Find(goMoveInfo_.target);
    TL_RETURN_IF(target);

    GameObject* targetParent = goMgr->Find(target->GetParentID());
    TL_RETURN_IF(target->GetParentID() != scene->GetRootGOID() || targetParent);

    parent->RemoveChild(source->GetID());

    if (goMoveInfo_.toBeChild) {
        target->AppendChild(source->GetID());
    } else {
        targetParent->SetChildToNext(target->GetID(), source->GetID());
    }
}

}  // namespace tl