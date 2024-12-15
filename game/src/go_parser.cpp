#include "go_parser.hpp"

#include "context.hpp"

namespace tl {

GameObjectParser::GameObjectParser(GameObjectManager& mgr) : mgr_{mgr} {}

GameObject* GameObjectParser::operator()(const tinyxml2::XMLElement& node) {
    return parseGORecurse(node);
}

GameObject* GameObjectParser::parseGORecurse(const tinyxml2::XMLElement& node) {
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

GameObject* GameObjectParser::parseGO(const tinyxml2::XMLElement& node) {
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
    TL_RETURN_NULL_IF_FALSE_LOGW(!err, "%s parse failed: %s", filename.c_str(),
                                 GetXMLErrStr(err));

    auto goElem = doc.FirstChildElement("gameobject");
    TL_RETURN_NULL_IF_FALSE_LOGE(goElem, "%s don't exists `gameobject` elem",
                                 attr->Value());

    GameObject* go = mgr_.Create();
    go->name = attr->Value();

    auto transformElem = goElem->FirstChildElement("transform");
    if (transformElem) {
        go->SetLocalTransform(parseTransform(*transformElem));
    }

    auto spriteElem = goElem->FirstChildElement("sprite");
    if (spriteElem) {
        go->sprite = parseSprite(*spriteElem);
    }

    auto tilemapElem = goElem->FirstChildElement("tilemap");
    if (tilemapElem) {
        go->tilemap = parseTileMap(*tilemapElem);
    }

    auto animatorElem = goElem->FirstChildElement("animator");
    if (animatorElem) {
        go->animator = parseAnimator(*animatorElem);
    }

    auto physicActorElem = goElem->FirstChildElement("physic_actor");
    if (physicActorElem) {
        go->physicActor = parsePhysicActor(*physicActorElem);
    }

    auto cameraElem = goElem->FirstChildElement("camera");
    if (cameraElem) {
        go->camera = parseCamera(*cameraElem);
    }
    
    auto roleElem = goElem->FirstChildElement("role");
    if (roleElem) {
        go->role = parseRole(*roleElem);
    }
    
    auto enableElem = goElem->FirstChildElement("enable");
    if (enableElem) {
        if (std::string_view{enableElem->GetText()} == "true") {
           go->enable = true; 
        } else {
            go->enable = false;
        }
    }

    return go;
}

TileMap* GameObjectParser::parseTileMap(const tinyxml2::XMLElement& elem) const {
    auto nameElem = elem.FirstChildElement("name");
    const char* name = nameElem->GetText();
    return Context::GetInst().tilemapMgr->Find(name);
}

Animator GameObjectParser::parseAnimator(const tinyxml2::XMLElement& elem) const {
    Animation* anim = Context::GetInst().animMgr->Find(elem.GetText());

    Animator animator;
    animator.animation = anim;

    if (!anim) {
        LOGW("animator %s not exists", elem.GetText());
    }

    auto rate = elem.FindAttribute("rate");
    animator.SetRate(rate ? rate->FloatValue() : 1.0);

    auto loop = elem.FindAttribute("loop");
    animator.SetLoop(loop ? loop->IntValue() : 0);

    return animator;
}

Transform GameObjectParser::parseTransform(const tinyxml2::XMLElement& elem) const {
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

Sprite GameObjectParser::parseSprite(const tinyxml2::XMLElement& elem) const {
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
    sprite.enable = enable;

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

PhysicActor GameObjectParser::parsePhysicActor(const tinyxml2::XMLElement& elem) const {
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
    
    if (auto node = elem.FirstChildElement("filter")) {
        float value;
        ParseFloat(node->GetText(), &value, 1);
        actor.filter = value;
    }

    return actor;
}

Camera GameObjectParser::parseCamera(const tinyxml2::XMLElement& elem) const {
    Camera camera;
    camera.enable = true;
    if (auto offsetNode = elem.FirstChildElement("offset")) {
        ParseFloat(offsetNode->GetText(), (float*)&camera.offset, 2);
    }
    if (auto scaleNode = elem.FirstChildElement("scale")) {
        ParseFloat(scaleNode->GetText(), (float*)&camera.scale, 2);
    }
    return camera;
}

const RoleConfig& GameObjectParser::parseRole(const tinyxml2::XMLElement& elem) const {
    return Context::GetInst().roleConfigMgr->Find(elem.GetText());
}

}