#include "scene.hpp"
#include "common.hpp"
#include "context.hpp"
#include "macro.hpp"
#include "physics.hpp"
#include "tilemap.hpp"

namespace tl {

Scene SceneManager::null_;

Scene::Scene(const std::string& filename) {
    void* fileContent = SDL_LoadFile(filename.c_str(), nullptr);
    if (!fileContent) {
        LOGE("can't load %s", filename.c_str());
        return;
    }

    tinyxml2::XMLDocument doc;
    auto err = doc.Parse((const char*)fileContent);
    TL_RETURN_IF_FALSE_LOGW(!err, "%s load failed", filename.c_str());

    load(doc);
}

GameObjectManager& Scene::GetGOMgr() {
    return const_cast<GameObjectManager&>(std::as_const(*this).GetGOMgr());
}

const GameObjectManager& Scene::GetGOMgr() const {
    return goMgr_;
}

void Scene::load(tinyxml2::XMLDocument& doc) {
    auto root = GetGOMgr().Create();
    root->name = "root";
    rootGO_ = root->GetID();

    auto sceneElem = doc.FirstChildElement("scene");
    TL_RETURN_IF_FALSE_LOGW(sceneElem, "`scene` elem not exists");

    auto node = sceneElem->FirstChild();
    while (node) {
        auto elem = node->ToElement();
        node = node->NextSibling();

        TL_CONTINUE_IF_FALSE(elem);
        GameObject* go = parseGORecurse(*elem);
        TL_CONTINUE_IF_FALSE(go);

        root->AppendChild(*go);
    }
}

GameObject* Scene::parseGORecurse(const tinyxml2::XMLElement& node) {
    GameObject* go = parseGO(node);
    TL_RETURN_NULL_IF_FALSE(go);

    int elemCount = node.ChildElementCount();
    if (elemCount > 0) {
        const tinyxml2::XMLNode* child = node.FirstChild();
        while (child) {
            auto elem = child->ToElement();
            child = child->NextSibling();
            TL_CONTINUE_IF_FALSE(elem);

            GameObject* childGO = parseGORecurse(*elem);
            if (childGO) {
                go->AppendChild(*childGO);
            }
        }
    }

    return go;
}

GameObject* Scene::parseGO(const tinyxml2::XMLElement& node) {
    const tinyxml2::XMLAttribute* attr = node.FindAttribute("path");
    TL_RETURN_NULL_IF_FALSE(attr);

    std::string filename =
        std::string("assets/gpa/go/") + attr->Value() + ".xml";
    void* fileContent = SDL_LoadFile(filename.c_str(), nullptr);
    if (!fileContent) {
        LOGE("can't load %s", filename.c_str());
        return nullptr;
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.Parse((const char*)fileContent);
    TL_RETURN_NULL_IF_FALSE_LOGW(!err, "%s parse failed", filename.c_str());

    auto goElem = doc.FirstChildElement("gameobject");
    TL_RETURN_NULL_IF_FALSE_LOGE(goElem, "%s don't exists `gameobject` elem",
                                 attr->Value());

    GameObject* go = GetGOMgr().Create();
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

    auto animationElem = goElem->FirstChildElement("animation");
    if (animationElem) {
        go->animator = parseAnimator(*animationElem);
    }

    auto physicActorElem = goElem->FirstChildElement("physic_actor");
    if (physicActorElem) {
        go->physicActor = parsePhysicActor(*physicActorElem);
    }

    return go;
}

TileMap* Scene::parseTileMap(const tinyxml2::XMLElement& elem) const {
    auto nameElem = elem.FirstChildElement("name");
    const char* name = nameElem->GetText();
    return Context::GetInst().tilemapMgr->Find(name);
}

Animator Scene::parseAnimator(const tinyxml2::XMLElement& elem) const {
    Animation* anim = Context::GetInst().animMgr->Find(elem.GetText());

    Animator animator;
    animator.animation = anim;

    if (!anim) {
        LOGW("animation %s not exists", elem.GetText());
    }

    auto rate = elem.FindAttribute("rate");
    animator.SetRate(rate ? rate->FloatValue() : 1.0);

    auto loop = elem.FindAttribute("loop");
    animator.SetLoop(loop ? loop->IntValue() : 0);

    return animator;
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
    bool enable = true;
    if (enableElem) {
        auto text = enableElem->GetText();
        if (strcmp(text, "true") == 0) {
            enable = true;
        } else if (strcmp(text, "false") == 0) {
            enable = false;
        } else {
            LOGW("sprite `is_enable` elem can't parse");
        }
    }
    sprite.isEnable = enable;

    auto colorElem = elem.FirstChildElement("color");
    if (colorElem) {
        ParseFloat(colorElem->GetText(), (float*)&sprite.color, 4);
    }

    auto textureElem = elem.FirstChildElement("texture");
    if (textureElem) {
        const char* filename = textureElem->GetText();
        Texture* texture = Context::GetInst().textureMgr->Find(filename);
        if (!texture) {
            LOGW("texture %s not exists", filename);
        } else {
            sprite.SetTexture(*texture);
        }
    } else {
        auto textElem = elem.FirstChildElement("text");
        if (textElem) {
            auto nameAttr = textElem->FindAttribute("font");
            const char* name = nullptr;
            const char* text = "";
            if (nameAttr) {
                name = nameAttr->Value();
            }

            auto sizeAttr = textElem->FindAttribute("size");
            uint8_t size = 16;
            if (sizeAttr) {
                size = sizeAttr->Int64Value();
            }

            text = textElem->GetText();
            auto font = Context::GetInst().fontMgr->Find(name);
            if (font) {
                sprite.SetFontTexture(FontTexture(*font, text, size));
            }
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

PhysicActor Scene::parsePhysicActor(const tinyxml2::XMLElement& elem) const {
    PhysicActor actor;
    actor.enable = false;
    auto aabbElem = elem.FirstChildElement("aabb");
    if (aabbElem) {
        float values[4] = {0};
        ParseFloat(aabbElem->GetText(), values, 4);
        actor.enable = true;
        actor.shape.type = Shape::Type::AABB;
        actor.shape.aabb.center = Vec2{values[0], values[1]};
        actor.shape.aabb.halfSize = Vec2{values[2], values[3]};
    }

    auto circleElem = elem.FirstChildElement("circle");
    if (circleElem) {
        float values[3] = {0};
        ParseFloat(circleElem->GetText(), values, 3);
        actor.enable = true;
        actor.shape.type = Shape::Type::Circle;
        actor.shape.circle.center = Vec2{values[0], values[1]};
        actor.shape.circle.radius = values[2];
    }

    if (auto node = elem.FirstChildElement("trigger")) {
        actor.isTrigger = true;
    }

    return actor;
}

void Scene::clear() {
    auto& goMgr = Context::GetInst().sceneMgr->GetCurScene().GetGOMgr();
    deleteGORecurse(rootGO_);
}

void Scene::deleteGORecurse(GameObjectID id) {
    auto& goMgr = Context::GetInst().sceneMgr->GetCurScene().GetGOMgr();
    auto go = goMgr.Find(id);
    TL_RETURN_IF_FALSE(go);

    for (auto child : go->GetChildren()) {
        deleteGORecurse(id);
    }

    goMgr.Destroy(id);
}

GameObjectID Scene::GetRootGOID() const {
    return rootGO_;
}

GameObject* Scene::GetRootGO() {
    return Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(
        GetRootGOID());
}

void Scene::RegisterLevel(std::unique_ptr<Level>&& level) {
    level_ = std::move(level);
}

void Scene::Update() {
    if (level_) {
        level_->Update();
    }
    updateGOTransformRecurse(nullptr, *GetRootGO(), false);
    Context::GetInst().physicsScene->ClearActors();
    addGOs2PhysicsScene();
    Context::GetInst().physicsScene->Update(1);
    updateGOTransformRecurse(nullptr, *GetRootGO(), true);
    updateGO(nullptr, GetRootGO());
}

void Scene::updateGO(GameObject* parent, GameObject* go) {
    if (go->animator.animation) {
        go->animator.Update(1);
        if (go->animator.IsPlaying()) {
            syncAnim2GO(*go);
        }
    }

    drawTileMap(*go);
    drawSprite(*go);

    for (GameObjectID id : go->GetChildren()) {
        GameObject* child =
            Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(id);
        if (child) {
            updateGO(go, child);
        }
    }
}

void Scene::updateGOTransformRecurse(GameObject* parent, GameObject& child,
                                     bool syncPhysics) {
    if (syncPhysics && child.physicActor) {
        Vec2 dir = child.physicActor.shape.GetCenter() * child.transform.scale;
        child.globalTransform_.position = child.physicActor.GetCollideShape().GetCenter() - Rotate(dir, child.transform.rotation);
        if (parent) {
            child.transform = CalcLocalTransformToParent(
                parent->GetGlobalTransform(), child.GetGlobalTransform());
        } else {
            child.transform = child.globalTransform_;
        }
    } else {
        if (parent) {
            child.globalTransform_ = CalcTransformFromParent(
                parent->GetGlobalTransform(), child.transform);
        } else {
            child.globalTransform_ = child.transform;
        }
    }

    for (auto c : child.GetChildren()) {
        GameObject* go =
            Context::GetInst().sceneMgr->GetCurScene().GetGOMgr().Find(c);
        TL_CONTINUE_IF_FALSE(go);

        updateGOTransformRecurse(&child, *go, syncPhysics);
    }
}

void Scene::drawSprite(const GameObject& go) const {
    if (!go.sprite.isEnable || !go.sprite) {
        return;
    }

    Context::GetInst().renderer->DrawTexture(
        *go.sprite.GetTexture(), go.sprite.GetRegion(), go.GetGlobalTransform(),
        go.sprite.anchor, go.sprite.flip, go.sprite.color);
}

void Scene::addGOs2PhysicsScene() {
    for (auto&& [_, go] : GetGOMgr().GetAllGO()) {
        Context::GetInst().physicsScene->MarkAsPhysics(&go);
    }
}

void Scene::syncAnim2GO(GameObject& go) {
    TL_RETURN_IF_FALSE(go.animator);

    auto& floatTrack = go.animator.floatTracks_;
    for (int i = 0; i < floatTrack.size(); i++) {
        TL_CONTINUE_IF_FALSE(
            !go.animator.animation->floatTracks[i].keyframes.empty());

        auto& track = floatTrack[i];
        auto bind = static_cast<FloatBindPoint>(i);
        switch (bind) {
            case FloatBindPoint::GORotation:
                go.transform.rotation = track.curData;
                break;
        }
    }

    auto& vec2Track = go.animator.vec2Tracks_;
    for (int i = 0; i < vec2Track.size(); i++) {
        TL_CONTINUE_IF_FALSE(
            !go.animator.animation->vec2Tracks[i].keyframes.empty());

        auto bind = static_cast<Vec2BindPoint>(i);
        auto& track = vec2Track[i];
        switch (bind) {
            case Vec2BindPoint::GOPosition:
                go.transform.position = track.curData;
                break;
            case Vec2BindPoint::GOScale:
                go.transform.scale = track.curData;
                break;
        }
    }

    auto& textureTrack = go.animator.textureTracks_;
    for (int i = 0; i < textureTrack.size(); i++) {
        TL_CONTINUE_IF_FALSE(
            !go.animator.animation->textureTracks[i].keyframes.empty());

        auto bind = static_cast<TextureBindPoint>(i);
        auto& track = textureTrack[i];
        switch (bind) {
            case TextureBindPoint::Sprite:
                go.sprite.SetTexture(*track.curData);
                break;
        }
    }

    auto& rectTrack = go.animator.rectTracks_;
    for (int i = 0; i < rectTrack.size(); i++) {
        TL_CONTINUE_IF_FALSE(
            !go.animator.animation->rectTracks[i].keyframes.empty());

        auto bind = static_cast<RectBindPoint>(i);
        auto& track = rectTrack[i];
        switch (bind) {
            case RectBindPoint::Sprite:
                go.sprite.SetRegion(track.curData);
                break;
        }
    }
}

void Scene::drawTileMap(const GameObject& go) const {
    TL_RETURN_IF_FALSE(go.tilemap);

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
            TL_CONTINUE_IF_FALSE(tile && tile->tilesetIndex);
            const TileSet& tileset = map.GetTileSet(tile->tilesetIndex.value());

            Transform localTransform;
            localTransform.position = Vec2(x, y) * tileset.tileSize;
            Transform globalTrans =
                CalcTransformFromParent(trans, localTransform);

            renderer->DrawTexture(*tileset.texture, tile->region, globalTrans,
                                  Vec2{}, tile->flip, Color::White);
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
                              Vec2{0, 1}, obj.flip, Color::White);
    }
}

SceneManager::SceneManager() {
    void* fileContent = SDL_LoadFile("assets/scenes.xml", nullptr);
    if (!fileContent) {
        LOGE("can't load assets/scenes.xml");
        Context::GetInst().Exit();
        return;
    }

    tinyxml2::XMLDocument doc;
    auto err = doc.Parse((const char*)fileContent);
    TL_RETURN_IF_FALSE_LOGE(!err, "assets/scenes.xml parse failed");

    auto scenesElem = doc.FirstChildElement("scenes");
    TL_RETURN_IF_FALSE_LOGE(scenesElem, "don't exists `scenes`");

    auto startupElem = scenesElem->FirstChildElement("startup");
    if (!startupElem) {
        Context::GetInst().Exit();
        TL_RETURN_IF_FALSE_LOGE(startupElem, "don't exists startup scene");
    }
    const char* startupName = startupElem->GetText();

    auto sceneElem = scenesElem->FirstChildElement("scene");
    TL_RETURN_IF_FALSE_LOGE(sceneElem, "don't exists `scene`");

    auto node = sceneElem->FirstChild();
    while (node) {
        auto elem = node->ToElement();
        node = node->NextSibling();

        TL_CONTINUE_IF_FALSE(elem);

        auto attr = elem->FindAttribute("path");
        TL_CONTINUE_IF_FALSE(attr && attr->Value());

        std::string name = attr->Value();
        Scene scene("assets/gpa/scene/" + name + ".xml");

        auto& emplacedScene =
            sceneMap_.emplace(name, std::move(scene)).first->second;
    }

    ChangeScene(startupName);
}

Scene* SceneManager::Find(const std::string& name) {
    auto it = sceneMap_.find(name);
    if (it == sceneMap_.end()) {
        return nullptr;
    }
    return &it->second;
}

Scene& SceneManager::GetCurScene() {
    return curScene_ ? *curScene_ : null_;
}

void SceneManager::Update() {
    if (curScene_) {
        curScene_->Update();
    }
}

bool SceneManager::ChangeScene(const std::string& name) {
    auto it = sceneMap_.find(name);
    if (it != sceneMap_.end()) {
        changeDstScene_ = &it->second;
        return true;
    }
    return false;
}

void SceneManager::PostUpdate() {
    if (changeDstScene_) {
        if (curScene_) {
            Level* level = curScene_->GetLevel();
            if (level) {
                level->Quit();
            }
        }

        curScene_ = changeDstScene_;
        changeDstScene_ = nullptr;

        Level* level = curScene_->GetLevel();
        if (curScene_ && level) {
            if (!level->IsInited()) {
                level->Init();
                level->isInited_ = true;
            }
            level->Enter();
        }
    }
}

}  // namespace tl
