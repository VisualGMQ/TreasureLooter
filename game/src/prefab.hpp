#pragma once
#include "gameobject.hpp"

namespace tl {

class Prefab {
public:
    explicit Prefab(GameObject*);
    Prefab() = default;
    GameObject* Instantiate(GameObjectManager&);

    operator bool() const noexcept;
    
private:
    GameObject* go_{};
    void clone(GameObject& src, GameObject& dst);
    GameObject* instantiateRecur(GameObjectManager& mgr, GameObject& src);
};

class PrefabManager {
public:
    Prefab Load(const std::string& name);
    Prefab Find(const std::string& name);
    const GameObject* Find(std::string_view name) const;
    void Destroy(std::string_view name);
    void Clear();
    
private:
    GameObjectManager goMgr_;
};

}
