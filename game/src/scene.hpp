#pragma once
#include "gameobject.hpp"

namespace tl {

class Scene {
public:
    Scene(const std::string& filename);
    void Update();

    operator bool() const;
    GameObjectID GetRootGOID() const;
    GameObject* GetRootGO();

    const std::vector<GameObjectID>& GetAllGOID() const;

private:
    std::vector<GameObjectID> goList_;

    void load(tinyxml2::XMLDocument& doc);
    void clear();
    void parseGOInfo(tinyxml2::XMLElement& elem);

    GameObject* parseGORecurse(const tinyxml2::XMLElement& node);
    GameObject* parseGO(const tinyxml2::XMLElement& node) const;
    Sprite parseSprite(const tinyxml2::XMLElement& elem) const;
    Transform parseTransform(const tinyxml2::XMLElement& elem) const;
    TileMap* parseTileMap(const tinyxml2::XMLElement& elem) const;

    void drawSprite(const GameObject&) const;
    void syncAnim2GO(GameObject&);
    void updateGO(GameObject* parent, GameObject* go);
    void drawTileMap(const GameObject&) const;
    void drawTileLayer(const Transform&, const TileMap&, const TileLayer&) const;
    void drawObjectLayer(const Transform&, const TileMap&, const ObjectLayer&) const;
};

class SceneManager {
public:
    SceneManager();
    Scene* Find(const std::string& name);
    Scene* GetCurScene();

    void Update();
    void PostUpdate();
    bool ChangeScene(const std::string& name);

    auto& GetAllScenes() const { return sceneMap_; }

private:
    std::unordered_map<std::string, Scene> sceneMap_;
    Scene* curScene_ = nullptr;
    Scene* changeDstScene_ = nullptr;
};

}