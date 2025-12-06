#pragma once

#include "engine/context.hpp"

class CommonContext;

using GizmoID = uint32_t;
constexpr GizmoID InvalidGizmoID = 0;

class Gizmo {
public:
    Gizmo(GizmoID id) : m_id{id} {}

    virtual ~Gizmo() = default;

    GizmoID GetID() const;
    const Vec2& GetPosition() const;
    void MoveTo(const Vec2& position);

    /**
     * set position without trigger fetch state
     */
    void SetPosition(const Vec2& position);

    /**
     * fetch position
     * @return true if gizmo moved
     * @param[out] position  gizmo position
     */
    bool fetchPosition(Vec2& position);
    virtual bool IsPointIn(const Vec2&) const = 0;
    void Show();
    void Hide();
    bool IsVisiable() const;
    void Hovering(bool is_hovering);
    bool IsHovering() const;
    virtual void Draw(CommonContext&) = 0;

private:
    bool m_visiable = false;
    bool m_hovering = false;
    bool m_moved = false;
    GizmoID m_id;
    Vec2 m_position;
};

class AxisGizmo : public Gizmo {
public:
    AxisGizmo(GizmoID);

    void SetLen(float);
    void SetWidth(float);

    bool IsPointIn(const Vec2&) const override;
    void Draw(CommonContext&) override;

private:
    float m_len = 100;
    float m_width = 10;
};

class ButtonGizmo : public Gizmo {
public:
    ButtonGizmo(GizmoID);
    void SetLen(float);

    bool IsPointIn(const Vec2&) const override;
    void Draw(CommonContext&) override;

private:
    float m_len = 20;
};

class GizmoManager {
public:
    AxisGizmo& CreateAxisGizmo();
    ButtonGizmo& CreateButtonGizmo();

    void Remove(GizmoID);

    void Update(CommonContext&);
    void Draw(CommonContext&);

private:
    std::unordered_map<GizmoID, std::unique_ptr<Gizmo>> m_gizmoes;
    GizmoID m_cur_id = InvalidGizmoID + 1;
    GizmoID m_hovering_gizmo = InvalidGizmoID;

    GizmoID genID();
};
