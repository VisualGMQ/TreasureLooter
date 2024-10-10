#pragma once
#include "level.hpp"
#include "game_controller.hpp"

namespace tl {

class TestMoveAndSlideLevel : public Level {
public:
    void Init() override;
    void Enter() override;
    void Quit() override {}
    void Update() override {}
};

class MoveController: public GameController {
public:
    void Update() override;
};



}  // namespace tl