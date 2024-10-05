#include "scene.hpp"
#include "common.hpp"
#include "context.hpp"
#include "macro.hpp"
#include "tilemap.hpp"


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

    auto sceneElem = doc.FirstChildElement("scene");
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

GameObject* Scene::parseGORecurse(const tinyxml2::XMLElement& node) {
    GameObject* go = parseGO(node);
    TL_RETURN_NULL_IF(go);
    goList_.push_back(go->GetID());

    int elemCount = node.ChildElementCount();
    if (elemCount > 0) {
        const tinyxml2::XMLNode* child = node.FirstChild();
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

GameObject* Scene::parseGO(const tinyxml2::XMLElement& node) const {
    const tinyxml2::XMLAttribute* attr = node.FindAttribute("path");
    TL_RETURN_NULL_IF(attr);

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.LoadFile(
        (std::string("assets/gpa/go/") + attr->Value() + ".xml").c_str());
    TL_RETURN_NULL_IF_LOGW(!err, "%s not exists", attr->Value());

    auto goElem = doc.FirstChildElement("gameobject");
    TL_RETURN_NULL_IF_LOGE(goElem, "%s don't exists `gameobject` elem",
                           attr->Value());

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

    auto tilemapElem = goElem->FirstChildElement("tilemap");
    if (tilemapElem) {
        go->tilemap = parseTileMap(*tilemapElem);
    }

    return go;
}

TileMap* Scene::parseTileMap(const tinyxml2::XMLElement& elem) const {
    auto nameElem = elem.FirstChildElement("name");
    const char* name = nameElem->GetText();
    return Context::GetInst().tilemapMgr->Find(name);
}

Transform Scene::parseTransform(const tinyxml2::XMLElement& elem) const {
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

Sprite Scene::parseSprite(const tinyxml2::XMLElement& elem) const {
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
        Texture* texture = Context::GetInst().textureMgr->Get(filename);
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

    auto flipElem = elem.FirstChildElement("flip");
    if (flipElem) {
        std::string_view text = flipElem->GetText();
        if (text == "vertical") {
            sprite.flip = Flip::Vertical;
        } else if (text == "horizontal") {
            sprite.flip = Flip::Horizontal;
        } else if (text == "both") {
            sprite.flip |= Flip::Vertical;
            sprite.flip |= Flip::Horizontal;
        }
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
    drawTileMap(*go);
    drawSprite(*go);

    for (GameObjectID id : go->GetChildren()) {
        GameObject* child = Context::GetInst().goMgr->Find(id);
        if (child) {
            updateGO(go, child);
        }
    }
}

void Scene::drawSprite(const GameObject& go) const {
    if (!go.sprite.isEnable || !go.sprite) {
        return;
    }

    Context::GetInst().renderer->DrawTexture(
        *go.sprite.GetTexture(), go.sprite.GetRegion(), go.GetGlobalTransform(),
        go.sprite.anchor, go.sprite.flip);
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

void Scene::drawTileMap(const GameObject& go) const {
    TL_RETURN_IF(go.tilemap);

    for (auto& layer : go.tilemap->GetLayers()) {
        if (layer->GetType() == MapLayerType::Object) {
            drawObjectLayer(go.globalTransform_, *go.tilemap,
                            (ObjectLayer&)*layer);
        } else if (layer->GetType() == MapLayerType::Tiles) {
            drawTileLayer(go.globalTransform_, *go.tilemap, (TileLayer&)*layer);
        }
    }
}

void Scene::drawTileLayer(const Transform& trans, const TileMap& map,
                          const TileLayer& layer) const {
    auto& renderer = Context::GetInst().renderer;
    for (uint32_t y = 0; y < layer.GetSize().h; y++) {
        for (uint32_t x = 0; x < layer.GetSize().w; x++) {
            const Tile* tile = layer.GetTile(x, y);
            TL_CONTINUE_IF(tile && tile->tilesetIndex);
            const TileSet& tileset = map.GetTileSet(tile->tilesetIndex.value());

            Transform localTransform;
            localTransform.position = Vec2(x, y) * tileset.tileSize;
            Transform globalTrans = CalcTransformFromParent(trans, localTransform);

            renderer->DrawTexture(*tileset.texture, tile->region, globalTrans,
                                  Vec2{}, tile->flip);
        }
    }
}

void Scene::drawObjectLayer(const Transform& trans, const TileMap& tilemap,
                            const ObjectLayer& layer) const {
    auto& renderer = Context::GetInst().renderer;
    for (auto& obj : layer.tileObjects) {
        Transform localTransform;
        localTransform.position = obj.position;
        localTransform.scale = obj.size / obj.region.size;
        Transform globalTrans = CalcTransformFromParent(trans, localTransform);
        renderer->DrawTexture(*obj.tileset->texture, obj.region, globalTrans,
                              Vec2{0, 1}, obj.flip);
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
        auto scene =
            std::make_unique<Scene>("assets/gpa/scene/" + name + ".xml");
        TL_CONTINUE_IF(scene && *scene);

        auto& emplacedScene =
            sceneMap_.emplace(name, std::move(scene)).first->second;

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