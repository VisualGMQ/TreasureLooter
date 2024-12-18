#pragma once
#include "gameobject.hpp"
#include "level.hpp"

namespace tl {

class Scene {
public:
    Scene() = default;  // trivial constructor to construct a NULL-like object
    Scene(const std::string& filename);
    void RegisterLevel(std::unique_ptr<Level>&& level);

    Level* GetLevel() const { return level_.get(); }

    void Update();

    GameObjectID GetRootGOID() const;
    GameObject* GetRootGO();
    GameObjectManager& GetGOMgr();
    const GameObjectManager& GetGOMgr() const;

private:
    std::unique_ptr<Level> level_;
    GameObjectID rootGO_;
    GameObjectManager goMgr_;

    void load(tinyxml2::XMLDocument& doc);
    void clear();

    void syncAnim2GO(GameObject&);
    void addGOs2PhysicsScene();

    void updateGO(GameObject* go);
    void drawSprite(const GameObject&) const;
    void drawTileMap(const GameObject&) const;
    void drawTileLayer(const TileMap&,
                       const TileLayer&) const;
    void drawObjectLayer(const Transform&, const TileMap&,
                         const ObjectLayer&) const;
    void deleteGORecurse(GameObjectID go);
};

class SceneManager {
public:
    SceneManager();
    Scene* Find(const std::string& name);
    Scene& GetCurScene();

    void Update();
    void PostUpdate();
    bool ChangeScene(const std::string& name);

    auto& GetAllScenes() const { return sceneMap_; }

private:
    std::unordered_map<std::string, Scene> sceneMap_;
    static Scene null_;
    Scene* curScene_ = nullptr;
    Scene* changeDstScene_ = nullptr;
};

}