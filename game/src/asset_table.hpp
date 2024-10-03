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
};

}