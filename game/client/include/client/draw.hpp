#pragma once
#include "common/entity.hpp"

class DrawCommandSubmitter {
public:
    void Submit();
    void SubmitUI();

private:
    void submit(LogicEntity root_entity);
    void submitRecursive(LogicEntity entity);
    
    void submitUI(LogicEntity root_entity);
    void submitUIRecursive(LogicEntity entity);
};
