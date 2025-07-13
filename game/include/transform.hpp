#pragma once
#include "manager.hpp"
#include "math.hpp"

struct Transform {
    Pose m_pose; // local pose
    Pose m_global_pose;

    Transform() = default;
    explicit Transform(const Pose& pose): m_pose{pose} {}
};

class TransformManager: public ComponentManager<Transform> {};