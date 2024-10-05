#pragma once
#include "pch.hpp"
#include "math.hpp"
#include "renderer.hpp"
#include "texture.hpp"

namespace tl {

struct TileSet {
    operator bool() const;

    Vec2 tileSize;
    uint32_t margin = 0;
    uint32_t padding = 0;
    Texture* texture = nullptr;
    std::string name;
};

enum class MapLayerType {
    Object,
    Tiles,
};

struct MapLayer {
    explicit MapLayer(MapLayerType type) : type_{type} {}
    virtual ~MapLayer() = default;

    MapLayerType GetType() const { return type_; }

private:
    MapLayerType type_;
};

struct ObjectLayer: public MapLayer {
    struct TileObject {
        Vec2 position;
        Vec2 size;
        Rect region;
        const TileSet* tileset = nullptr;
        Flags<Flip> flip = Flip::None;
    };

    std::vector<Ellipse> ellipses;
    std::vector<Rect> rects;
    std::vector<Vec2> points;
    std::vector<Polygon> polygons;
    std::vector<Polyline> polylines;
    std::vector<TileObject> tileObjects;

    ObjectLayer() : MapLayer{MapLayerType::Object} {}
};

struct Tile {
    std::optional<size_t >tilesetIndex;
    Rect region;
    Flags<Flip> flip;

    operator bool() const {
        return tilesetIndex.has_value();
    }
};

class TileLayer: public MapLayer {
public:
    TileLayer(uint32_t w, uint32_t h);

    void SetTile(uint32_t x, uint32_t y, const Tile&);
    const Tile* GetTile(uint32_t x, uint32_t y) const;
    Vec2 GetSize() const;

private:
    // Row-major
    std::vector<Tile> tiles_;
    uint32_t w_ = 0, h_ = 0;
};

class TileMap {
public:
    TileMap(const std::string& filename);
    auto& GetLayers() const { return layers_; }
    auto& GetTileSet(uint32_t idx) const { return tileSets_[idx]; }

private:
    std::vector<std::unique_ptr<MapLayer>> layers_;
    std::vector<TileSet> tileSets_;

    std::unique_ptr<ObjectLayer> parseObjectLayer(const tson::Layer&) const;
    std::unique_ptr<TileLayer> parseTileLayer(const tson::Layer&) const;
    TileSet parseTileSet(const tson::Tileset&) const;
};

struct TileMapManager {
public:
    TileMap* Load(const std::string& filename, const std::string& name);
    TileMap* Find(const std::string& name);

private:
    std::unordered_map<std::string, std::unique_ptr<TileMap>> tilemaps_;
};

}  // namespace tl