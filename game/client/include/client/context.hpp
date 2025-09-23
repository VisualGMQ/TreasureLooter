#pragma once
#include "engine/context.hpp"
#include "client/motor.hpp"

class MotorManager;

class GameContext : public CommonContext {
public:
    static void Init();
    static void Destroy();
    static GameContext& GetInst();

    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;
    GameContext(GameContext&&) = delete;
    GameContext& operator=(GameContext&&) = delete;
    ~GameContext() override;

    void Initialize() override;
    void Shutdown() override;

    void Update() override;
    std::unique_ptr<MotorManager> m_motor_manager;

private:
    static std::unique_ptr<GameContext> instance;

    void logicUpdate(TimeType elapse);
    void logicPostUpdate(TimeType elapse);
    void renderUpdate(TimeType elapse);

    GameContext() = default;
};

#define GAME_CONTEXT ::GameContext::GetInst()
