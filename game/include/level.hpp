#pragma once
#include "animation.hpp"
#include "image.hpp"
#include "timer.hpp"

class Level {
public:
    Level();
    virtual ~Level() = default;
    virtual void OnInit() = 0;
    virtual void OnUpdate(TimeType) = 0;
    virtual void OnQuit() = 0;

    Entity GetRootEntity() const;

private:
    Entity m_root_entity{};

    void initRootEntity();
};

// some game related logic
class GameLevel: public Level {
public:
    void OnInit() override;
    void OnUpdate(TimeType) override;
    void OnQuit() override;

private:
    enum class WalkDirection {
        Up,
        Left,
        Right,
        Down,
    };

    AnimationHandle m_walk_left;
    AnimationHandle m_walk_right;
    AnimationHandle m_walk_up;
    AnimationHandle m_walk_down;
    ImageHandle m_image_sheet;
    Entity m_player_entity;

    WalkDirection m_walk_direction = WalkDirection::Down;
};