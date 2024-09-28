#include "inspector.hpp"
#include "context.hpp"
#include "go_hierarchy_watcher.hpp"


namespace tl {

void Inspector::Update() {
    auto& ctx = Context::GetInst();
    GameObjectID id = ctx.debugMgr->hierarchyWatcher->GetSelected();

    if (ImGui::Begin("inspector")) {
        GameObject* go = ctx.goMgr->Find(id);
        if (go) {
            updateName(*go);
            updateTransform(*go);
            updateSprite(*go);
            updateAnimator(*go);
        }
    }
    ImGui::End();
}

void Inspector::updateName(GameObject& go) {
    char buf[128] = {0};
    strcpy(buf, go.name.c_str());
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
    ImGui::InputText("name", buf, sizeof(buf), flags);

    if (ImGui::IsItemActivated()) {
        lastGOName_ = go.name;
    }

    go.name = buf;
    if (go.name.empty()) {
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            go.name = lastGOName_;
        }
    }
}

void Inspector::updateTransform(GameObject& go) {
    ImGui::PushID("transform");
    if (ImGui::CollapsingHeader("transform")) {
        updateTransformGeneric(go.transform);
    }
    ImGui::PopID();
}

void Inspector::updateTransformGeneric(Transform& transform) {
        ImGui::DragFloat2("position", (float*)&transform.position);
        ImGui::DragFloat2("scale", (float*)&transform.scale);
        ImGui::DragFloat("rotation", &transform.rotation, 1.0, 0.0, 0.0,
                         "%.3f(deg)");
}

void Inspector::updateSprite(GameObject& go) {
    if (!go.sprite) {
        return;
    }

    ImGui::PushID("sprite");
    if (ImGui::CollapsingHeader("sprite")) {
        ImGui::DragFloat2("anchor", (float*)&go.sprite.anchor);

        Rect region = go.sprite.GetRegion();
        ImGui::DragFloat4("region", (float*)(&region));
        go.sprite.SetRegion(region);

        auto texture = go.sprite.GetTexture();
        if (texture) {
            ImVec2 size{texture->GetSize().w, texture->GetSize().h};
            ImGui::Image(texture->texture_, size);
        }
    }
    ImGui::PopID();
}

void Inspector::updateAnimator(GameObject& go) {}

}  // namespace tl