#pragma once
#include "engine/manager.hpp"

class Script {
public:
    virtual ~Script() = default;

    virtual void OnInit() = 0;
    virtual void OnEnter() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnQuit() = 0;
    virtual void OnDestroy() = 0;
};

class ScriptManager: public ComponentManager<Script> {
public: 
};