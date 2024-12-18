#include "inspector.hpp"
#include "context.hpp"
#include "go_hierarchy_watcher.hpp"
#include "macro.hpp"

namespace tl {

void Inspector::Update() {
    auto& goMgr = Context::GetInst().sceneMgr->GetCurScene().GetGOMgr();
    GameObjectID id = Context::GetInst().debugMgr->hierarchyWatcher->GetSelected();

    if (ImGui::Begin("inspector")) {
        GameObject* go = goMgr.Find(id);
        if (go) {
            updateName(go->name);
            ImGui::Checkbox("enable", &go->enable);
            
            Transform transform = go->GetLocalTransform(); 
            updateTransform("transform", transform);
            if (transform != go->GetLocalTransform()) {
                go->SetLocalTransform(transform);
            }
            
            updateTransform("global transform", go->GetGlobalTransform());
            if (go->sprite) {
                updateSprite(go->sprite);
            }
            if (go->animator) {
                updateAnimator(go->animator);
            }
            if (go->tilemap) {
                updateTileMap(*go->tilemap);
            }
            if (go->physicActor) {
                updatePhysicActor(go->physicActor);
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

void Inspector::updateTransform(const std::string& title, Transform& transform) {
    ImGui::PushID(title.c_str());
    if (ImGui::CollapsingHeader(title.c_str())) {
        updateTransformGeneric(transform);
    }
    ImGui::PopID();
}

void Inspector::updateTransform(const std::string& title, const Transform& transform) {
    ImGui::PushID(title.c_str());
    if (ImGui::CollapsingHeader(title.c_str())) {
        updateTransformGeneric(transform);
    }
    ImGui::PopID();
}

void Inspector::updateTransformGeneric(Transform& transform) {
    ImGui::DragFloat2("position", (float*)&transform.position);
    ImGui::DragFloat2("scale", (float*)&transform.scale, 0.1);
    transform.scale.x = transform.scale.x == 0 ? 0.000001f : transform.scale.x;
    transform.scale.y = transform.scale.y == 0 ? 0.000001f : transform.scale.y;
    ImGui::DragFloat("rotation", &transform.rotation, 1.0, 0.0, 0.0,
                     "%.3f(deg)");
}

void Inspector::updateTransformGeneric(const Transform& transform) {
    ImGui::BeginDisabled(true);
    ImGui::DragFloat2("position", (float*)&transform.position);
    ImGui::DragFloat2("scale", (float*)&transform.scale, 0.1);
    ImGui::DragFloat("rotation", (float*)&transform.rotation, 1.0, 0.0, 0.0,
                     "%.3f(deg)");
    ImGui::EndDisabled();
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
        bool flipV = (sprite.flip & Flip::Vertical) != Flip::None;
        bool flipH = (sprite.flip & Flip::Horizontal) != Flip::None;
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
    if (ImGui::CollapsingHeader("animator")) {
        ImGui::Text("status: %s", animator.IsPlaying() ? "playing" : "pausing");
        ImGui::Text("%lu/%lu", (TimeType)animator.GetCurTime(), animator.animation->GetMaxTime());

        if (ImGui::Button("play")) {
            animator.Play();
        }
        ImGui::SameLine();

        if (ImGui::Button("pause")) {
            animator.Pause();
        }

        ImGui::SameLine();
        if (ImGui::Button("stop")) {
            animator.Stop();
        }

        float rate  = animator.GetRate();
        ImGui::DragFloat("rate", &rate, 0.1);
        animator.SetRate(rate);

        int loop = animator.GetLoop();
        ImGui::DragInt("loop", &loop, 1, -1, INT_MAX);
        animator.SetLoop(loop);
    }
}

void Inspector::updateTileMap(TileMap& tilemap) {
    if (ImGui::CollapsingHeader("tileMap")) {
    }
}

void Inspector::updatePhysicActor(PhysicActor& actor) {
    if (ImGui::CollapsingHeader("PhysicActor")) {
        ImGui::BeginDisabled(!actor.enable);
        switch (actor.shape.type) {
            case Shape::Type::Unknown:
                ImGui::Text("unknown shape type");
                break;
            case Shape::Type::AABB:
                ImGui::Text("AABB");
                ImGui::DragFloat2("center", (float*)&actor.shape.aabb.center,
                                  0.1);
                ImGui::DragFloat2("halfSize",
                                  (float*)&actor.shape.aabb.halfSize, 0.1, 0,
                                  FLT_MAX);
                break;
            case Shape::Type::Circle:
                ImGui::Text("Circle");
                ImGui::DragFloat("radius", &actor.shape.circle.radius, 0.1, 0, FLT_MAX);
                ImGui::DragFloat2("center", (float*)&actor.shape.circle.center, 0.1);
                break;
        }
        ImGui::EndDisabled();
    }
}

}  // namespace tl