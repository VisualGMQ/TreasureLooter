#pragma once
#include "pch.hpp"
#include "math.hpp"
#include "physics.hpp"
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

class ObjectLayer;
class TileLayer;

struct MapLayer {
    explicit MapLayer(MapLayerType type) : type_{type} {}
    virtual ~MapLayer() = default;

    MapLayerType GetType() const { return type_; }

    ObjectLayer* AsObjectLayer() {
        return const_cast<ObjectLayer*>(std::as_const(*this).AsObjectLayer());
    }
    TileLayer* AsTileLayer() {
        return const_cast<TileLayer*>(std::as_const(*this).AsTileLayer());
    }
    const ObjectLayer* AsObjectLayer() const;
    const TileLayer* AsTileLayer() const;

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
        const tson::PropertyCollection* properties = nullptr;
    };

    std::vector<Ellipse> ellipses;
    std::vector<Rect> rects;
    std::vector<Point> points;
    std::vector<Polygon> polygons;
    std::vector<Polyline> polylines;
    std::vector<TileObject> tileObjects;

    ObjectLayer() : MapLayer{MapLayerType::Object} {}
};

struct TileMapCollision {
    Shape shape;
    uint32_t collisionGroup = 0;
    bool isTrigger = false;
};

struct Tile {
    std::string name;
    std::optional<size_t >tilesetIndex;
    Rect region;
    Flags<Flip> flip;
    PhysicActor actor;

    operator bool() const {
        return tilesetIndex.has_value();
    }

    void UpdateTransform(const Transform& parentTrans, const Vec2& grid,
                         const Vec2& tileSize);
    const Transform& GetGlobalTransform() const;

private:
    Transform globalTransform_;
};

class TileMap;

class TileLayer: public MapLayer {
public:
    TileLayer(uint32_t w, uint32_t h);

    void SetTile(uint32_t x, uint32_t y, const Tile&);
    const Tile* GetTile(uint32_t x, uint32_t y) const;
    Tile* GetTile(uint32_t x, uint32_t y);
    Vec2 GetSize() const;

    const auto& GetAllTiles() const { return tiles_; }

    void UpdateTransform(const Transform& parentTrans, const TileMap*);

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

    auto& GetLayers() { return layers_; }

    auto& GetTileSet(uint32_t idx) { return tileSets_[idx]; }

    void UpdateTransform(const Transform& parentTrans);

private:
    std::vector<std::unique_ptr<MapLayer>> layers_;
    std::vector<TileSet> tileSets_;
    std::unordered_map<uint32_t, TileMapCollision> collisionMap_;

    std::unique_ptr<ObjectLayer> parseObjectLayer(const tson::Layer&) const;
    std::unique_ptr<TileLayer> parseTileLayer(const tson::Layer&) const;
    TileSet parseTileSet(const tson::Tileset&);
};

class TileMapManager {
public:
    TileMap* Load(const std::string& filename, const std::string& name);
    TileMap* Find(const std::string& name);

private:
    std::unordered_map<std::string, TileMap> tilemaps_;
};

}  // namespace tl