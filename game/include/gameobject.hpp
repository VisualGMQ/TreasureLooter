#pragma once
#include "sprite.hpp"
#include <vector>

struct GameObject {
    friend class Context;

    Pose m_pose;  // local pose
    Sprite m_sprite;

    std::vector<GameObject> m_children;

    const Pose& GetGlobalPose() const { return m_global_pose; }

private:
    Pose m_global_pose;
};