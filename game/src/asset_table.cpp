#include "asset_table.hpp"
#include "context.hpp"
#include "log.hpp"
#include "macro.hpp"

namespace tl {

AssetTable::AssetTable() {
    void* fileContent =  SDL_LoadFile("assets/assets.xml", nullptr);
    if (!fileContent) {
        LOGE("can't load assets/assets.xml");
        Context::GetInst().Exit();
        return;
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.Parse((const char*)fileContent);
    if (err) {
        LOGE("assets.xml parse failed");
        Context::GetInst().Exit();
        return;
    }

    auto assetsElem = doc.FirstChildElement("assets");
    TL_RETURN_IF_LOGE(assetsElem, "assets.xml don't has `assets` element");

    // parse textures
    tinyxml2::XMLElement* textureList = assetsElem->FirstChildElement("texture");
    TL_RETURN_IF(textureList);

    auto node = textureList->FirstChild();
    while (node) {
        auto elem = node->ToElement();
        node = node->NextSibling();
        TL_CONTINUE_IF(elem);
        parseTexture(*elem);
    }

    // parse tile map
    tinyxml2::XMLElement* tilemapList = assetsElem->FirstChildElement("tilemap");
    TL_RETURN_IF(tilemapList);

    node = tilemapList->FirstChild();
    while (node) {
        auto elem = node->ToElement();
        node = node->NextSibling();
        TL_CONTINUE_IF(elem);
        parseTilemap(*elem);
    }
}

void AssetTable::parseTexture(tinyxml2::XMLElement& node) {
    auto attr = node.FindAttribute("path");
    std::string name = attr->Value();
    auto path = "assets/image/" + name;

    Context::GetInst().textureMgr->Load(path, name);
}

void AssetTable::parseTilemap(tinyxml2::XMLElement& node) {
    auto attr = node.FindAttribute("path");
    std::string name = attr->Value();
    auto path = "assets/tilemap/" + name + ".json";

    Context::GetInst().tilemapMgr->Load(path, name);
}


}  // namespace tl