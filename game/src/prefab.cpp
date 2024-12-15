#include "prefab.hpp"

#include "go_parser.hpp"

namespace tl {

Prefab::Prefab(GameObject* go) : go_{go} {}

GameObject* Prefab::Instantiate(GameObjectManager& mgr) {
    TL_RETURN_NULL_IF_FALSE_LOGW(go_, "can't instantiate null prefab");
    return instantiateRecur(mgr, *go_);
}

Prefab::operator bool() const noexcept {
    return go_;
}

void Prefab::clone(GameObject& src, GameObject& dst) {
    dst.name = src.name;
    dst.localTransform_ = src.localTransform_;
    dst.camera = src.camera;
    dst.role = src.role;
    dst.tilemap = src.tilemap;
    dst.physicActor = src.physicActor;
    dst.animator = src.animator;

    dst.sprite.SetRegion(src.sprite.GetRegion());
    if (dst.sprite.IsTexture() && src.sprite.GetTexture()) {
        dst.sprite.SetTexture(*src.sprite.GetTexture());
    } else if (dst.sprite.IsText() && dst.sprite.GetFont()) {
        dst.sprite.SetFontTexture(FontTexture{*src.sprite.GetFont(),
                                              src.sprite.GetText(),
                                              src.sprite.GetFontSize()});
    }
    dst.sprite.anchor = src.sprite.anchor;
    dst.sprite.color = src.sprite.color;
    dst.sprite.flip = src.sprite.flip;
    dst.sprite.enable = src.sprite.enable;
}

GameObject* Prefab::instantiateRecur(GameObjectManager& mgr, GameObject& src) {
    GameObject* go = mgr.Create();
    clone(src, *go);

    for (auto& id : src.GetChildren()) {
        GameObject* child = mgr.Find(id);
        TL_CONTINUE_IF_FALSE_WITH_LOGW(child, "instantiate invalid go");
        GameObject* newChild = instantiateRecur(mgr, *child);
        go->AppendChild(*newChild);
    }

    return go;
}

Prefab PrefabManager::Load(const std::string& name) {
    GameObjectParser parser(goMgr_);

    std::string filename = "assets/gpa/go/" + name + ".xml";
    tinyxml2::XMLDocument doc;
    void* fileContent = SDL_LoadFile(filename.c_str(), nullptr);
    if (!fileContent) {
        LOGE("can't load %s", filename.c_str());
        return Prefab{};
    }

    auto err = doc.Parse((const char*)fileContent);
    TL_RETURN_DEFAULT_IF_FALSE_LOGW(!err, "%s load failed", filename.c_str());

    auto elem = doc.FirstChildElement("element");
    GameObject* go = parser(*elem);
    go->name = name;
    return Prefab{go};
}

Prefab PrefabManager::Find(const std::string& name) {
    return Prefab{goMgr_.Find(name)};
}

const GameObject* PrefabManager::Find(std::string_view name) const {
    return goMgr_.Find(name);
}

void PrefabManager::Destroy(std::string_view name) {
    auto go = goMgr_.Find(name);
    TL_RETURN_IF_FALSE(go);

    goMgr_.Destroy(go->GetID());
}

void PrefabManager::Clear() {
    goMgr_.Clear();
}

}  // namespace tl
