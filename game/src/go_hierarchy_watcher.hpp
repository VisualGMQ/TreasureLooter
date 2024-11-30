#pragma once
#include "pch.hpp"
#include "gameobject.hpp"

namespace tl {

class GOHierarchyWatcher {
public:
    void Update();

    GameObjectID GetSelected() const { return selectedGO_; }

private:
    void updateRecursive(GameObject& go, int& id, bool isParentOpen,
                         bool isParentDragging, bool isDisabled);
    GameObjectID selectedGO_;
    int curSelectFpsLimit_ = 0;

    struct GOMoveInfo {
        GameObjectID target;
        GameObjectID source;
        bool toBeChild = false;

        void Reset() {
            target = GameObjectID{};
            source = GameObjectID{};
        }

        operator bool() const { return source && target; }
    } goMoveInfo_;
    GameObjectID draggingGOID_;
    bool shouldChangeDraggingState_ = false;

    void applyGOMove();
};

}