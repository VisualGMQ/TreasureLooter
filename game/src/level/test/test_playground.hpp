#pragma once
#include "common.hpp"
#include "level.hpp"

namespace tl {

class TestLevel : public Level {
public:
    using Level::Level;
    void Init() override {}
    void Enter() override;
    void Quit() override {}
    void Update() override;

private:
    GameObjectID selectedGO_;
};

}  // namespace tl