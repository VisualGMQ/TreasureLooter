#pragma once
#include "common/manager.hpp"
#include "common/math.hpp"

class TransformManager: public ComponentManager<Transform, LogicEntity> {};

class PresentTransformManager
    : public ComponentManager<Transform, PresentEntity> {};
