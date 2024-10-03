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

    GameObject* parseGORecurse(tinyxml2::XMLElement& node);
    GameObject* parseGO(tinyxml2::XMLElement& node);
    Sprite parseSprite(tinyxml2::XMLElement& elem);
    Transform parseTransform(tinyxml2::XMLElement& elem);

    void drawSprite(GameObject&);
    void syncAnim2GO(GameObject&);
    void updateGO(GameObject* parent, GameObject* go);
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
    std::unordered_map<std::string, std::unique_ptr<Scene>> sceneMap_;
    Scene* curScene_ = nullptr;
    Scene* changeDstScene_ = nullptr;
};

}