#pragma once
#include "engine/entity.hpp"

class DrawCommandSubmitter {
public:
    void Submit();
    void SubmitUI();

private:
    void submit(Entity root_entity);
    void submitRecursive(Entity entity);
    
    void submitUI(Entity root_entity);
    void submitUIRecursive(Entity entity);
};
