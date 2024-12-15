#include "scene.hpp"
#include "common.hpp"
#include "context.hpp"
#include "go_parser.hpp"
#include "macro.hpp"
#include "physics.hpp"
#include "profile.hpp"
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

EventManager& Scene::GetEventMgr() {
    return eventMgr_;
}

const EventManager& Scene::GetEventMgr() const {
    return eventMgr_;
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
        GameObjectParser parser{goMgr_};
        GameObject* go = parser(*elem);
        TL_CONTINUE_IF_FALSE(go);

        root->AppendChild(*go);
    }

    if (auto cameraElem = sceneElem->FirstChildElement("camera")) {
        auto attr = cameraElem->FindAttribute("name");
        if (attr) {
            GameObject* go = GetGOMgr().Find(attr->Value());
            if (go) {
                Context::GetInst().cameraGOID = go->GetID();
            }
        }
    }
}

void Scene::clear() {
    deleteGORecurse(rootGO_);
}

void Scene::deleteGORecurse(GameObjectID id) {
    auto& goMgr = GetGOMgr();
    auto go = goMgr.Find(id);
    TL_RETURN_IF_FALSE(go);

    for (auto child : go->GetChildren()) {
        deleteGORecurse(child);
    }

    goMgr.Destroy(id);
}

GameObjectID Scene::GetRootGOID() const {
    return rootGO_;
}

GameObject* Scene::GetRootGO() {
    return GetGOMgr().Find(GetRootGOID());
}

void Scene::RegisterLevel(std::unique_ptr<Level>&& level) {
    level_ = std::move(level);
}

void Scene::Update() {
    PROFILE_SECTION_BEGIN("level update");
    if (level_) {
        level_->Update();
    }
    PROFILE_SECTION_END();

    GetRootGO()->UpdateTransform({}, false);
    Context::GetInst().physicsScene->ClearActors();
    addGOs2PhysicsScene();
    Context::GetInst().physicsScene->Update(1);
    GetRootGO()->UpdateTransform({}, true);

    Context::GetInst().renderer->SetCamera(Context::GetInst().GetCamera());
    updateGO(GetRootGO());

    eventMgr_.Update();
}

void Scene::updateGO(GameObject* go) {
    PROFILE_FUNC();
    TL_RETURN_IF_FALSE(go->enable);
    
    if (go->animator.animation) {
        go->animator.Update(1);
        if (go->animator.IsPlaying()) {
            syncAnim2GO(*go);
        }
    }

    if (go->camera) {
        go->camera.Update(*go);
    }

    drawTileMap(*go);
    drawSprite(*go);

    for (GameObjectID id : go->GetChildren()) {
        GameObject* child = GetGOMgr().Find(id);
        if (child) {
            updateGO(child);
        }
    }
}

void Scene::drawSprite(const GameObject& go) const {
    PROFILE_FUNC();
    
    if (!go.sprite.enable || !go.sprite) {
        return;
    }

    Context::GetInst().renderer->DrawTexture(
        *go.sprite.GetTexture(), go.sprite.GetRegion(), go.GetGlobalTransform(),
        go.sprite.anchor, go.sprite.flip, go.sprite.color);
}

void Scene::addGOs2PhysicsScene() {
    PROFILE_FUNC();
    for (auto&& [_, go] : GetGOMgr().GetAllGO()) {
        Context::GetInst().physicsScene->MarkAsPhysics(&go);
    }
}

void Scene::syncAnim2GO(GameObject& go) {
    PROFILE_FUNC();
    
    TL_RETURN_IF_FALSE(go.animator);

    auto& floatTrack = go.animator.floatTracks_;
    for (int i = 0; i < floatTrack.size(); i++) {
        TL_CONTINUE_IF_FALSE(
            !go.animator.animation->floatTracks[i].keyframes.empty());

        auto& track = floatTrack[i];
        auto bind = static_cast<FloatBindPoint>(i);
        switch (bind) {
            case FloatBindPoint::GORotation:
                go.SetLocalRotation(track.curData);
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
                go.SetLocalPosition(track.curData);
                break;
            case Vec2BindPoint::GOScale:
                go.SetLocalScale(track.curData);
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
    PROFILE_FUNC();
    
    TL_RETURN_IF_FALSE(go.tilemap);

    for (auto& layer : go.tilemap->GetLayers()) {
        if (layer->GetType() == MapLayerType::Object) {
            drawObjectLayer(go.globalTransform_, *go.tilemap,
                            *layer->AsObjectLayer());
        } else if (layer->GetType() == MapLayerType::Tiles) {
            drawTileLayer(*go.tilemap, *layer->AsTileLayer());
        }
    }
}

void Scene::drawTileLayer(const TileMap& map,
                          const TileLayer& layer) const {
    auto& renderer = Context::GetInst().renderer;
    auto halfWinSize = Context::GetInst().window->GetSize() * 0.5;

    for (uint32_t y = 0; y < layer.GetSize().h; y++) {
        for (uint32_t x = 0; x < layer.GetSize().w; x++) {
            const Tile* tile = layer.GetTile(x, y);
            TL_CONTINUE_IF_FALSE(tile && tile->tilesetIndex);

            auto& globalTransform = tile->GetGlobalTransform();
            AABB aabb{
                globalTransform.position -
                    Context::GetInst().GetCamera().GetGlobalOffset(),
                Vec2{std::abs(globalTransform.scale.x),
                     std::abs(globalTransform.scale.y)}
                *
                    tile->region.size
            };

            TL_CONTINUE_IF_FALSE(
                IsAABBOverlap(aabb, AABB{halfWinSize, halfWinSize}));
            
            const TileSet& tileset = map.GetTileSet(tile->tilesetIndex.value());
            renderer->DrawTexture(*tileset.texture, tile->region,
                                  tile->GetGlobalTransform(), Vec2{},
                                  tile->flip, Color::White);
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

        sceneMap_.emplace(name, std::move(scene));
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
    PROFILE_FUNC();
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
    PROFILE_FUNC();
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
