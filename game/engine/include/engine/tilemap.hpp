#pragma once
#include "engine/flag.hpp"
#include "engine/image.hpp"
#include "engine/manager.hpp"
#include "engine/path.hpp"
#include "engine/physics.hpp"
#include "schema/common.hpp"
#include "schema/flip.hpp"
#include "tmxlite/Layer.hpp"
#include "tmxlite/Map.hpp"

class TilemapTileLayer;
class TilemapDefinition;

constexpr std::string_view TilemapPropertyName_EnableYSort = "enable_y_sort";

constexpr std::array<std::string_view, static_cast<size_t>(RenderLayer::RenderLayerCount)> TilemapRenderLayerNameMap = {
    "land",
    "arch",
};

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

    bool IsEnableYSort() const;
    RenderLayer GetRenderLayer() const;

protected:
    bool m_enable_y_sort = false;
    RenderLayer m_render_layer = RenderLayer::Default;

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

class Tileset;

struct Tile {
    ImageHandle m_image;
    Region m_region;
    uint32_t m_id;
    Rect m_collision_rect;
    Vec2 m_tile_size;
};

class Tileset {
public:
    Tileset(const tmx::Tileset&);

    const Tile& GetTile(uint32_t gid) const;

    bool HasTile(uint32_t gid) const;
    const Vec2& GetTileSize() const;

private:
    void parse(const tmx::Tileset& tileset);

    ImageHandle m_image;
    uint32_t m_margin;
    uint32_t m_spacing;
    std::vector<std::uint32_t> m_tile_index;
    std::vector<Tile> m_tiles;
    uint32_t m_firstgid;
    uint32_t m_lastgid;
    Vec2 m_tile_size;
};

class Tilemap {
public:
    explicit Tilemap(const Path& filename);

    auto& GetLayers() const { return m_layers; }

    auto& GetTileset() const { return m_tilesets; }

    const Tile* GetTile(uint32_t gid) const;

    const Vec2& GetTileSize() const;

    const Path& GetFilename() const;

private:
    void parse(const Path& filename);

    std::vector<std::unique_ptr<TilemapLayer>> m_layers;
    std::vector<Tileset> m_tilesets;
    Vec2 m_tile_size;
    Path m_filename;
};

using TilemapHandle = Handle<Tilemap>;

class TilemapManager : public AssetManagerBase<Tilemap> {
public:
    TilemapHandle Load(const Path& filename, bool force = false) override;
};


class TilemapComponent {
public:
    TilemapComponent(Entity, const TilemapDefinition&);

    [[nodiscard]] TilemapHandle GetHandle() const;

    const PhysicsScene::TilemapCollision* GetTilemapCollision() const;

private:
    TilemapHandle m_handle;
    PhysicsScene::TilemapCollision* m_tilemap_collision{};
};

class TilemapComponentManager : public ComponentManager<TilemapComponent> {
public:
    void Update();

private:
    void drawTilemap(const TilemapComponent& tilemap);
};
