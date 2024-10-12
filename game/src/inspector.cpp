#include "inspector.hpp"
#include "context.hpp"
#include "go_hierarchy_watcher.hpp"
#include "macro.hpp"

namespace tl {

void Inspector::Update() {
    auto& ctx = Context::GetInst();
    GameObjectID id = ctx.debugMgr->hierarchyWatcher->GetSelected();

    if (ImGui::Begin("inspector")) {
        GameObject* go = ctx.goMgr->Find(id);
        if (go) {
            updateName(go->name);
            updateTransform(go->transform);
            if (go->sprite) {
                updateSprite(go->sprite);
            }
            if (go->animator) {
                updateAnimator(go->animator);
            }
            if (go->tilemap) {
                updateTileMap(*go->tilemap);
            }
        }
    }
    ImGui::End();
}

void Inspector::updateName(const std::string& name) const {
    char buf[128] = {0};
    strcpy(buf, name.c_str());
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_ReadOnly;
    ImGui::InputText("name", buf, sizeof(buf), flags);
}

void Inspector::updateTransform(Transform& transform) {
    ImGui::PushID("transform");
    if (ImGui::CollapsingHeader("transform")) {
        updateTransformGeneric(transform);
    }
    ImGui::PopID();
}

void Inspector::updateTransformGeneric(Transform& transform) {
    ImGui::DragFloat2("position", (float*)&transform.position);
    ImGui::DragFloat2("scale", (float*)&transform.scale, 0.1);
    ImGui::DragFloat("rotation", &transform.rotation, 1.0, 0.0, 0.0,
                     "%.3f(deg)");
}

void Inspector::updateSprite(Sprite& sprite) {
    ImGui::PushID("sprite");
    if (ImGui::CollapsingHeader("sprite")) {
        // display anchor
        ImGui::DragFloat2("anchor", (float*)&sprite.anchor);

        // display region
        Rect region = sprite.GetRegion();
        ImGui::DragFloat4("region", (float*)(&region));
        sprite.SetRegion(region);

        // display color
        ImGui::ColorEdit4("color", &sprite.color.r, ImGuiColorEditFlags_Float);

        // display flip
        bool flipV = sprite.flip & Flip::Vertical;
        bool flipH = sprite.flip & Flip::Horizontal;
        ImGui::Checkbox("Flip Horizontal", &flipH);
        ImGui::Checkbox("Flip Vertical", &flipV);

        sprite.flip = Flip::None;
        if (flipV) {
            sprite.flip |= Flip::Vertical;
        }
        if (flipH) {
            sprite.flip |= Flip::Horizontal;
        }

        // display texture
        if (sprite.IsTexture()) {
            auto texture = sprite.GetTexture();
            if (texture) {
                ImVec2 size{texture->GetSize().w, texture->GetSize().h};
                ImGui::Image(texture->texture_, size);
            }
        }

        // show text properties
        if (sprite.IsText()) {
            ImGui::SeparatorText("text");
            auto text = sprite.GetText();
            char buf[2048] = {0};
            strcpy(buf, text.c_str());
            ImGui::InputTextMultiline("text", buf, sizeof(buf));
            if (buf != text) {
                sprite.SetText(buf);
            }
            int ptSize = sprite.GetFontSize();
            ImGui::DragInt("ptSize", &ptSize, 1, 2, 255);
            if (ptSize != sprite.GetFontSize()) {
                sprite.SetFontSize(ptSize);
            }
        }
    }
    ImGui::PopID();
}

void Inspector::updateAnimator(Animator& animator) {
    // TODO:
}

void Inspector::updateTileMap(TileMap& tilemap) {
    if (ImGui::CollapsingHeader("tileMap")) {
    }
}

}  // namespace tl