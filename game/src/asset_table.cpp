#include "asset_table.hpp"
#include "context.hpp"
#include "log.hpp"
#include "macro.hpp"

namespace tl {

AssetTable::AssetTable() {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.LoadFile("assets/assets.xml");
    if (err) {
        LOGE("assets.xml load failed");
        Context::GetInst().Exit();
        return;
    }

    // parse textures
    auto assetsElem = doc.FirstChildElement("assets");
    TL_RETURN_IF_LOGE(assetsElem, "assets.xml don't has `assets` element");

    tinyxml2::XMLElement* textureList = assetsElem->FirstChildElement("texture");
    TL_RETURN_IF(textureList);

    auto node = textureList->FirstChild();
    while (node) {
        auto elem = node->ToElement();
        node = node->NextSibling();
        TL_CONTINUE_IF(elem);
        parseTexture(*elem);
    }
}



void AssetTable::parseTexture(tinyxml2::XMLElement& node) {
    auto attr = node.FindAttribute("path");
    std::string path = attr->Value();
    path = "assets/image/" + path;

    Context::GetInst().textureMgr->Load(path);
}


}  // namespace tl