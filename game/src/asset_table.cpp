#include "asset_table.hpp"
#include "context.hpp"
#include "log.hpp"
#include "macro.hpp"

namespace tl {

AssetTable::AssetTable() {
    void* fileContent = SDL_LoadFile("assets/assets.xml", nullptr);
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
    TL_RETURN_IF_FALSE_LOGE(assetsElem, "assets.xml don't has `assets` element");

    // parse textures
    tinyxml2::XMLElement* textureList =
        assetsElem->FirstChildElement("texture");
    TL_RETURN_IF_FALSE(textureList);

    auto node = textureList->FirstChild();
    while (node) {
        auto elem = node->ToElement();
        node = node->NextSibling();
        TL_CONTINUE_IF_FALSE(elem);
        parseTexture(*elem);
    }

    // parse tile map
    tinyxml2::XMLElement* tilemapList =
        assetsElem->FirstChildElement("tilemap");
    TL_RETURN_IF_FALSE(tilemapList);

    node = tilemapList->FirstChild();
    while (node) {
        auto elem = node->ToElement();
        node = node->NextSibling();
        TL_CONTINUE_IF_FALSE(elem);
        parseTilemap(*elem);
    }

    // parse font
    tinyxml2::XMLElement* fontList = assetsElem->FirstChildElement("font");
    TL_RETURN_IF_FALSE(fontList);

    node = fontList->FirstChild();
    while (node) {
        auto elem = node->ToElement();
        node = node->NextSibling();
        TL_CONTINUE_IF_FALSE(elem);
        parseFont(*elem);
    }

    // parse animation
    tinyxml2::XMLElement* animList = assetsElem->FirstChildElement("animation");
    TL_RETURN_IF_FALSE(animList);

    node = animList->FirstChild();
    while (node) {
        auto elem = node->ToElement();
        node = node->NextSibling();
        TL_CONTINUE_IF_FALSE(elem);
        parseAnimation(*elem);
    }

    // parse audio
    tinyxml2::XMLElement* audioList = assetsElem->FirstChildElement("audio");
    TL_RETURN_IF_FALSE(audioList);

    node = audioList->FirstChild();
    while (node) {
        auto elem = node->ToElement();
        node = node->NextSibling();
        TL_CONTINUE_IF_FALSE(elem);

        bool isMusic = strcmp(elem->Name(), "music") == 0;

        TL_CONTINUE_IF_FALSE(isMusic || strcmp(elem->Name(), "sound") == 0);
        parseAudio(*elem, isMusic);
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

void AssetTable::parseFont(tinyxml2::XMLElement& node) {
    auto attr = node.FindAttribute("path");
    std::string name = attr->Value();
    TL_RETURN_IF_FALSE(!name.empty());
    auto path = "assets/font/" + name;

    Context::GetInst().fontMgr->Load(path, name);
}

void AssetTable::parseAnimation(tinyxml2::XMLElement& node) {
    auto attr = node.FindAttribute("path");
    std::string name = attr->Value();
    TL_RETURN_IF_FALSE(!name.empty());
    auto path = "assets/anim/" + name + ".xml";

    Context::GetInst().animMgr->Load(path, name);
}

void AssetTable::parseAudio(tinyxml2::XMLElement& node, bool isMusic) {
    auto attr = node.FindAttribute("path");
    std::string name = attr->Value();
    TL_RETURN_IF_FALSE(!name.empty());

    auto path = "assets/audio/" + name;
    if (isMusic) {
        Context::GetInst().audioMgr->LoadMusic(path, name);
    } else {
        Context::GetInst().audioMgr->LoadSound(path, name);
    }
}

}  // namespace tl