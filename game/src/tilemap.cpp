#include "tilemap.hpp"

#include "context.hpp"
#include "log.hpp"
#include "asset_manager.hpp"

// Bits on the far end of the 32-bit global tile ID are used for tile flags
const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x8;
const unsigned FLIPPED_VERTICALLY_FLAG = 0x4;
const unsigned FLIPPED_DIAGONALLY_FLAG = 0x2;
const unsigned ROTATED_HEXAGONAL_120_FLAG = 0x1;

TilemapLayer::TilemapLayer(Type type) : m_type{type} {}

const TilemapTileLayer* TilemapLayer::CastAsTiledLayer() const {
    if (m_type == Type::Tiled) {
        return static_cast<const TilemapTileLayer*>(this);
    }
    return nullptr;
}

TilemapLayer::Type TilemapLayer::GetType() const {
    return m_type;
}

TilemapTileLayer::TilemapTileLayer(const tmx::TileLayer& layer)
    : TilemapLayer(Type::Tiled) {
    parse(layer);
}

const TilemapTileLayer::Tile& TilemapTileLayer::GetTile(int x, int y) const {
    size_t idx = y * m_size.x + x;
    return m_tiles[idx];
}

const Vec2& TilemapTileLayer::GetSize() const {
    return m_size;
}

void TilemapTileLayer::parse(const tmx::TileLayer& layer) {
    auto size = layer.getSize();
    m_size.x = size.x;
    m_size.y = size.y;
    for (int y = 0; y < size.y; y++) {
        for (int x = 0; x < size.x; x++) {
            const auto idx = y * size.x + x;
            auto& tmx_tile = layer.getTiles()[idx];
            Tile tile;
            if (tmx_tile.flipFlags & FLIPPED_VERTICALLY_FLAG) {
                tile.m_flip |= Flip::Vertical;
            }
            if (tmx_tile.flipFlags & FLIPPED_HORIZONTALLY_FLAG) {
                tile.m_flip |= Flip::Horizontal;
            }
            tile.m_gid = tmx_tile.ID;
            m_tiles.push_back(tile);
        }
    }
}

Tileset::Tileset(const tmx::Tileset& tileset) {
    parse(tileset);
}

void Tileset::parse(const tmx::Tileset& tileset) {
    auto path = tileset.getImagePath();
    m_image =
        GAME_CONTEXT.m_assets_manager->GetManager<ImageHandle>().Load(
            path);
    m_margin = tileset.getMargin();
    m_spacing = tileset.getSpacing();
    m_firstgid = tileset.getFirstGID();
    m_lastgid = tileset.getLastGID();

    for (uint32_t i = 0; i < tileset.getTileCount(); i++) {
        auto tmx_tile = tileset.getTile(i + tileset.getFirstGID());
        Tile tile;
        tile.m_id = tmx_tile->ID;
        tile.m_image = m_image;
        tile.m_region.m_topleft.x = tmx_tile->imagePosition.x;
        tile.m_region.m_topleft.y = tmx_tile->imagePosition.y;
        tile.m_region.m_size.x = tmx_tile->imageSize.x;
        tile.m_region.m_size.y = tmx_tile->imageSize.y;
        m_tiles.push_back(tile);
    }
}

const Tile& Tileset::GetTile(uint32_t gid) const {
    auto idx = gid - m_firstgid;
    return m_tiles[idx];
}

bool Tileset::HasTile(uint32_t gid) const {
    return gid >= m_firstgid && gid < m_lastgid;
}

Tilemap::Tilemap(const Path& filename) : m_filename{filename} {
    parse(filename);
}

const Tile* Tilemap::GetTile(uint32_t gid) const {
    for (auto& tileset : m_tilesets) {
        if (tileset.HasTile(gid)) {
            return &tileset.GetTile(gid);
        }
    }
    return nullptr;
}

const Vec2& Tilemap::GetTileSize() const {
    return m_tile_size;
}

const Path& Tilemap::GetFilename() const {
    return m_filename;
}

void Tilemap::parse(const Path& filename) {
    tmx::Map map;
    if (!map.load(filename.string())) {
        LOGE("load tilemap {} failed", filename);
        return;
    }

    m_tile_size.w = map.getTileSize().x;
    m_tile_size.h = map.getTileSize().y;

    auto& layers = map.getLayers();
    for (auto& layer : layers) {
        if (layer->getType() == tmx::Layer::Type::Tile) {
            const auto& tile_layer = layer->getLayerAs<tmx::TileLayer>();
            m_layers.emplace_back(
                std::make_unique<TilemapTileLayer>(tile_layer));
        }
    }

    const auto& tilesets = map.getTilesets();
    for (const auto& tileset : tilesets) {
        m_tilesets.emplace_back(tileset);
    }
}

TilemapHandle TilemapManager::Load(const Path& filename) {
    return store(&filename, UUID::CreateV4(),
                 std::make_unique<Tilemap>(filename));
}

void TilemapComponentManager::Update() {
    for (auto& [entity, tilemap] : m_components) {
        auto transform = GAME_CONTEXT.m_transform_manager->Get(entity);
        if (!transform) {
            continue;
        }

        drawTilemap(*transform, tilemap);
    }
}

void TilemapComponentManager::drawTilemap(const Transform& transform,
                                          const TilemapHandle& tilemap) {
    auto& renderer = GAME_CONTEXT.m_renderer;
    for (auto& layer : tilemap->GetLayers()) {
        if (layer->GetType() == TilemapLayer::Type::Tiled) {
            auto tiled_layer = layer->CastAsTiledLayer();
            auto& size = tiled_layer->GetSize();
            for (size_t y = 0; y < size.y; y++) {
                for (size_t x = 0; x < size.x; x++) {
                    auto& layer_tile = tiled_layer->GetTile(x, y);
                    auto tile = tilemap->GetTile(layer_tile.m_gid);
                    if (!tile) {
                        continue;
                    }
                    Region dst_region = tile->m_region;
                    float scale =
                        std::max(transform.m_scale.x, transform.m_scale.y);
                    scale += 0.01;
                    dst_region.m_topleft =
                        transform.m_position +
                        Vec2(x, y) * tilemap->GetTileSize() * scale;
                    dst_region.m_size *= scale;
                    renderer->DrawImage(*tile->m_image, tile->m_region,
                                        dst_region, 0, {0, 0},
                                        layer_tile.m_flip);
                }
            }
        }
    }
}
