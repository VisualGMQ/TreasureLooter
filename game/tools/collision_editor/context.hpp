#pragma once

#include "engine/context.hpp"
#include "tool_context.hpp"
#include "gizmo.hpp"

class CollisionEditorContext : public ToolContext {
public:
    static void Init();
    static void Destroy();
    static CollisionEditorContext& GetInst();

    void Initialize() override;
    void Shutdown() override;

    void HandleEvents(const SDL_Event&) override;

    void LoadAsset(Path);

protected:
    void update() override;

private:
    enum class Mode {
        None,
        CCT,
        Trigger,
    } m_mode = Mode::None;

    using PhysicsVariant =
        std::variant<std::monostate, CCT, PhysicsActorInfo, Trigger>;

    static std::unique_ptr<CollisionEditorContext> instance;
    Entity m_entity = null_entity;
    PrefabHandle m_prefab;
    PhysicsVariant m_variant;

    GizmoManager m_gizmo_manager;

    struct RectGizmos {
        ButtonGizmo* m_up{};
        ButtonGizmo* m_down{};
        ButtonGizmo* m_left{};
        ButtonGizmo* m_right{};
    };

    AxisGizmo* m_axis_gizmo{};
    ButtonGizmo* m_center_gizmo{};
    RectGizmos m_rect_gizmo{};
    ButtonGizmo* m_radius_gizmo{};

    void showMainMenu();
    void saveAs();
    void changeAssetPathInTitle(const Path& path);
    void interactiveConfigPhysics();
    void interactiveConfigCCT();
    void interactiveConfigTrigger();
};

#define COLLISION_EDITOR_CONTEXT ::CollisionEditorContext::GetInst()
