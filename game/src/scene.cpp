#include "scene.hpp"
#include "macro.hpp"
#include "common.hpp"
#include "context.hpp"

namespace tl {

Scene::Scene(const std::string& filename) {
    tinyxml2::XMLDocument doc;
    auto err = doc.LoadFile(filename.c_str());
    TL_RETURN_IF_LOGW(!err, "%s load failed", filename.c_str());

    load(doc);
}

void Scene::load(tinyxml2::XMLDocument& doc) {
    auto root = Context::GetInst().goMgr->Create();
    root->name = "root";
    goList_.push_back(root->GetID());

    auto sceneElem =doc.FirstChildElement("scene");
    TL_RETURN_IF_LOGW(sceneElem, "`scene` elem not exists");

    auto node = sceneElem->FirstChild();
    while (node) {
        auto elem = node->ToElement();
        node = node->NextSibling();

        TL_CONTINUE_IF(elem);
        GameObject* go = parseGORecurse(*elem);
        TL_CONTINUE_IF(go);

        root->AppendChild(go->GetID());
    }
}

GameObject* Scene::parseGORecurse(tinyxml2::XMLElement& node) {
    GameObject* go = parseGO(node);
    TL_RETURN_NULL_IF(go);
    goList_.push_back(go->GetID());

    int elemCount = node.ChildElementCount();
    if (elemCount > 0) {
        tinyxml2::XMLNode* child = node.FirstChild();
        while (child) {
            auto elem = child->ToElement();
            child = child->NextSibling();
            TL_CONTINUE_IF(elem);

            GameObject* childGO = parseGORecurse(*elem);
            if (childGO) {
                go->AppendChild(childGO->GetID());
            }
        }
    }

    return go;
}

GameObject* Scene::parseGO(tinyxml2::XMLElement& node) {
    const tinyxml2::XMLAttribute* attr = node.FindAttribute("path");
    TL_RETURN_NULL_IF(attr);

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.LoadFile(
        (std::string("assets/gpa/go/") + attr->Value() + ".xml").c_str());
    TL_RETURN_NULL_IF_LOGW(!err, "%s not exists", attr->Value());

    auto goElem = doc.FirstChildElement("gameobject");
    TL_RETURN_NULL_IF_LOGE(goElem, "%s don't exists `gameobject` elem", attr->Value());

    GameObject* go = Context::GetInst().goMgr->Create();
    go->name = attr->Value();

    auto transformElem = goElem->FirstChildElement("transform");
    if (transformElem) {
        go->transform = parseTransform(*transformElem);
    }

    auto spriteElem = goElem->FirstChildElement("sprite");
    if (spriteElem) {
        go->sprite = parseSprite(*spriteElem);
    }

    return go;
}

Transform Scene::parseTransform(tinyxml2::XMLElement& elem) {
    Transform transform;

    auto posElem = elem.FirstChildElement("position");
    if (posElem) {
        std::string_view content = posElem->GetText();
        ParseFloat(content, (float*)&transform.position, 2);
    }

    auto scaleElem = elem.FirstChildElement("scale");
    if (scaleElem) {
        std::string_view content = scaleElem->GetText();
        ParseFloat(content, (float*)&transform.scale, 2);
    }

    auto rotElem = elem.FirstChildElement("rotation");
    if (scaleElem) {
        std::string_view content = rotElem->GetText();
        ParseFloat(content, (float*)&transform.rotation, 1);
    }

    return transform;
}

Sprite Scene::parseSprite(tinyxml2::XMLElement& elem) {
    Sprite sprite;

    auto anchorElem = elem.FirstChildElement("anchor");
    if (anchorElem) {
        ParseFloat(anchorElem->GetText(), (float*)&sprite.anchor, 2);
    }

    auto enableElem = elem.FirstChildElement("is_enable");
    if (enableElem) {
        auto text = enableElem->GetText();
        if (strcmp(text, "true") == 0) {
            sprite.isEnable = true;
        } else if (strcmp(text, "false") == 0) {
            sprite.isEnable = false;
        } else {
            LOGW("sprite `is_enable` elem can't parse");
        }
    }

    auto textureElem = elem.FirstChildElement("texture");
    if (textureElem) {
        const char* filename = textureElem->GetText();
        Texture* texture = Context::GetInst().textureMgr->Get(std::string("assets/image/") + filename);
        if (!texture) {
            LOGW("texture %s not exists", filename);
        } else {
            sprite.SetTexture(*texture);
        }
    }

    auto regionElem = elem.FirstChildElement("region");
    if (regionElem) {
        Rect region;
        ParseFloat(regionElem->GetText(), (float*)(&region), 4);
        sprite.SetRegion(region);
    }

    return sprite;
}

void Scene::clear() {
    auto& goMgr = Context::GetInst().goMgr;
    for (auto id : goList_) {
        goMgr->Destroy(id);
    }
}

Scene::operator bool() const {
    return !goList_.empty();
}

GameObjectID Scene::GetRootGOID() const {
    if (goList_.empty()) {
        return {};
    }

    return goList_[0];
}

GameObject* Scene::GetRootGO() {
    TL_RETURN_NULL_IF(!goList_.empty());
    return Context::GetInst().goMgr->Find(goList_[0]);
}

const std::vector<GameObjectID>& Scene::GetAllGOID() const {
    return goList_;
}

void Scene::Update() {
    TL_RETURN_IF(!goList_.empty());
    updateGO(nullptr, Context::GetInst().goMgr->Find(goList_[0]));
}

void Scene::updateGO(GameObject* parent, GameObject* go) {
    if (go->animator.animation) {
        go->animator.animation->Update(1);
    }
    syncAnim2GO(*go);
    if (!parent) {
        go->globalTransform_ = go->transform;
    } else {
        go->globalTransform_ =
            CalcTransformFromParent(parent->globalTransform_, go->transform);
    }
    drawSprite(*go);

    for (GameObjectID id : go->GetChildren()) {
        GameObject* child = Context::GetInst().goMgr->Find(id);
        if (child) {
            updateGO(go, child);
        }
    }
}

void Scene::drawSprite(GameObject& go) {
    if (!go.sprite.isEnable || !go.sprite) {
        return;
    }

    auto& globalTransform = go.GetGlobalTransform();

    Rect dstRect;
    Vec2 unsignedScale = globalTransform.scale;
    unsignedScale.x = std::abs(globalTransform.scale.x);
    unsignedScale.y = std::abs(globalTransform.scale.y);
    dstRect.size = unsignedScale * go.sprite.GetRegion().size;

    Vec2 xAxis = Rotate(Vec2::X_AXIS, globalTransform.rotation);
    Vec2 yAxis = Rotate(Vec2::Y_AXIS, globalTransform.rotation);
    int xSign = Sign(globalTransform.scale.x);
    int ySign = Sign(globalTransform.scale.y);
    Vec2 xOffset, yOffset;
    if (xSign > 0) {
        xOffset = -dstRect.size.w * go.sprite.anchor.x * xAxis;
    } else if (xSign < 0) {
        xOffset = -dstRect.size.w * (1.0 - go.sprite.anchor.x) * xAxis;
    }
    if (ySign > 0) {
        yOffset = -dstRect.size.h * go.sprite.anchor.y * yAxis;
    } else if (xSign < 0) {
        yOffset = -dstRect.size.h * (1.0 - go.sprite.anchor.y) * yAxis;
    }
    dstRect.position = globalTransform.position + xOffset + yOffset;

    Flags<Flip> flip = Flip::None;
    if (globalTransform.scale.x < 0) {
        flip |= Flip::Horizontal;
    }
    if (globalTransform.scale.y < 0) {
        flip |= Flip::Vertical;
    }

    Context::GetInst().renderer->DrawTexture(
        *go.sprite.GetTexture(), go.sprite.GetRegion(), dstRect,
        globalTransform.rotation, Vec2::ZERO, flip);
}

void Scene::syncAnim2GO(GameObject& go) {
    if (!go.animator) {
        return;
    }

    auto& floatTrack = go.animator.animation->GetFloatTracks();
    for (auto& [bind, track] : floatTrack) {
        switch (bind) {
            case FloatBindPoint::GORotation:
                go.transform.rotation = track.curData;
                break;
        }
    }

    auto& vec2Track = go.animator.animation->GetVec2Tracks();
    for (auto& [bind, track] : vec2Track) {
        switch (bind) {
            case Vec2BindPoint::GOPosition:
                go.transform.position = track.curData;
                break;
            case Vec2BindPoint::GOScale:
                go.transform.scale = track.curData;
                break;
        }
    }

    auto& textureTrack = go.animator.animation->GetTextureTracks();
    for (auto& [bind, track] : textureTrack) {
        switch (bind) {
            case TextureBindPoint::Sprite:
                go.sprite.SetTexture(*track.curData);
                break;
        }
    }

    auto& rectTrack = go.animator.animation->GetRectTracks();
    for (auto& [bind, track] : rectTrack) {
        switch (bind) {
            case RectBindPoint::Sprite:
                go.sprite.SetRegion(track.curData);
                break;
        }
    }
}

SceneManager::SceneManager() {
    tinyxml2::XMLDocument doc;
    auto err = doc.LoadFile("assets/scenes.xml");
    TL_RETURN_IF_LOGE(!err, "scenes.xml load failed");

    auto scenesElem = doc.FirstChildElement("scenes");
    TL_RETURN_IF_LOGE(scenesElem, "don't exists `scenes`");

    auto startupElem = scenesElem->FirstChildElement("startup");
    if (!startupElem) {
        Context::GetInst().Exit();
        TL_RETURN_IF_LOGE(startupElem, "don't exists startup scene");
    }
    const char* startupName = startupElem->GetText();

    auto sceneElem = scenesElem->FirstChildElement("scene");
    TL_RETURN_IF_LOGE(sceneElem, "don't exists `scene`");

    auto node = sceneElem->FirstChild();
    while (node) {
        auto elem = node->ToElement();
        node = node->NextSibling();

        TL_CONTINUE_IF(elem);

        auto attr = elem->FindAttribute("path");
        TL_CONTINUE_IF(attr && attr->Value());

        std::string name = attr->Value();
        auto scene = std::make_unique<Scene>("assets/gpa/scene/" + name + ".xml");
        TL_CONTINUE_IF(scene && *scene);

        auto& emplacedScene = sceneMap_.emplace(name, std::move(scene)).first->second;

        if (name == startupName) {
            curScene_ = emplacedScene.get();
        }
    }
}

Scene* SceneManager::Find(const std::string& name) {
    auto it = sceneMap_.find(name);
    if (it == sceneMap_.end()) {
        return nullptr;
    }
    return it->second.get();
}

Scene* SceneManager::GetCurScene() {
    return curScene_;
}

void SceneManager::Update() {
    if (curScene_) {
        curScene_->Update();
    }
}

bool SceneManager::ChangeScene(const std::string& name) {
    auto it = sceneMap_.find(name);
    if (it != sceneMap_.end()) {
        changeDstScene_ = it->second.get();
        return true;
    }
    return false;
}

void SceneManager::PostUpdate() {
    if (changeDstScene_) {
        curScene_ = changeDstScene_;
        changeDstScene_ = nullptr;
    }
}

}  // namespace tl