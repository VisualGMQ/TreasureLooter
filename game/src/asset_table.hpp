#pragma once
#include "pch.hpp"
#include "gameobject.hpp"

namespace tl {

class AssetTable {
public:
    AssetTable();

private:
    void parseTexture(tinyxml2::XMLElement&);
    void parseTilemap(tinyxml2::XMLElement&);
    void parseFont(tinyxml2::XMLElement&);
    void parseAnimation(tinyxml2::XMLElement&);
    void parseAudio(tinyxml2::XMLElement&, bool isMusic);
    void parseRoleConfig(tinyxml2::XMLElement&);
    void parsePrefab(tinyxml2::XMLElement&);
};

}