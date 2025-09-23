#pragma once
#include "engine/state_machine.hpp"
#include "client/motor.hpp"

class EnemyIdleState : public State<EnemyMotorContext> {
public:
    void OnEnter(EnemyMotorContext* state) override;

    void OnQuit(EnemyMotorContext* state) override {}

    void OnUpdate(EnemyMotorContext* state) override;
};

class EnemyMoveState : public State<EnemyMotorContext> {
public:
    void OnEnter(EnemyMotorContext* state) override;

    void OnQuit(EnemyMotorContext* state) override;

    void OnUpdate(EnemyMotorContext* state) override;
};
