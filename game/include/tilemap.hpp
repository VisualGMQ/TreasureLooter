#pragma once
#include "flag.hpp"
#include "image.hpp"
#include "manager.hpp"
#include "path.hpp"
#include "schema/flip.hpp"
#include "tmxlite/Layer.hpp"
#include "tmxlite/Map.hpp"
#include "tmxlite/ObjectGroup.hpp"
#include "tmxlite/TileLayer.hpp"

class TilemapTileLayer;

class TilemapLayer {
public:
    enum class Type {
        Tiled,
        Object,
        Image,
        Group,
    };

    TilemapLayer(Type);
    virtual ~TilemapLayer() = default;

    const TilemapTileLayer* CastAsTiledLayer() const;

    Type GetType() const;

private:
    Type m_type;
};

class TilemapTileLayer : public TilemapLayer {
public:
    struct Tile {
        uint32_t m_gid;
        Flags<Flip> m_flip = Flip::None;
    };
    
    explicit TilemapTileLayer(const tmx::TileLayer&);

    const Tile& GetTile(int x, int y) const;
    const Vec2& GetSize() const;

private:
    void parse(const tmx::TileLayer&);

    std::vector<Tile> m_tiles;
    Vec2 m_size;
};

struct Tile {
    ImageHandle m_image;
    Region m_region;
    uint32_t m_id;
};

class Tileset {
public:
    Tileset(const tmx::Tileset&);
    const Tile& GetTile(uint32_t gid) const;

    bool HasTile(uint32_t gid) const;

private:
    void parse(const tmx::Tileset& tileset);

    ImageHandle m_image;
    uint32_t m_margin;
    uint32_t m_spacing;
    std::vector<std::uint32_t> m_tile_index;
    std::vector<Tile> m_tiles;
    uint32_t m_firstgid;
    uint32_t m_lastgid;
};

class Tilemap {
public:
    explicit Tilemap(const Path& filename);

    auto& GetLayers() const { return m_layers; }

    auto& GetTileset() const { return m_tilesets; }

    const Tile* GetTile(uint32_t gid) const;

    const Vec2& GetTileSize() const { return m_tile_size; }

private:
    void parse(const Path& filename);

    std::vector<std::unique_ptr<TilemapLayer>> m_layers;
    std::vector<Tileset> m_tilesets;
    Vec2 m_tile_size;
};

using TilemapHandle = Handle<Tilemap>;

class TilemapManager: public AssetManagerBase<Tilemap> {
public: 
    TilemapHandle Load(const Path& filename) override;
};

class TilemapComponentManager: public ComponentManager<TilemapHandle> {
public:
    void Update();
    
private:
    void drawTilemap(const Transform& transform, const TilemapHandle& tilemap);
};