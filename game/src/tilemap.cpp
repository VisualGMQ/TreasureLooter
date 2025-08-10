#include "tilemap.hpp"

#include "asset_manager.hpp"
#include "context.hpp"
#include "log.hpp"

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
    m_image = GAME_CONTEXT.m_assets_manager->GetManager<Image>().Load(path);
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

        auto& objs = tmx_tile->objectGroup.getObjects();
        if (!objs.empty()) {
            auto& obj = objs[0];
            if (obj.getShape() == tmx::Object::Shape::Rectangle) {
                auto& aabb = obj.getAABB();
                tile.m_collision_rect.m_half_size.w = aabb.width * 0.5;
                tile.m_collision_rect.m_half_size.h = aabb.height * 0.5;
                tile.m_collision_rect.m_center =
                    Vec2{aabb.left, aabb.top} +
                    tile.m_collision_rect.m_half_size;
            } else {
                LOGW("tile collision only support rectangle");
            }
        }
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

TilemapComponent::TilemapComponent(Entity entity, TilemapHandle handle)
    : m_handle{handle} {
    if (!handle) {
        return;
    }

    auto transform = GAME_CONTEXT.m_transform_manager->Get(entity);
    auto& game_config = GAME_CONTEXT.GetGameConfig();
    auto& physics_scene = GAME_CONTEXT.m_physics_scene;
    for (auto& layer : m_handle->GetLayers()) {
        if (layer->GetType() == TilemapLayer::Type::Tiled) {
            auto tiled_layer = layer->CastAsTiledLayer();
            auto& size = tiled_layer->GetSize();
            for (size_t y = 0; y < size.y; y++) {
                for (size_t x = 0; x < size.x; x++) {
                    auto& layer_tile = tiled_layer->GetTile(x, y);
                    auto tile = m_handle->GetTile(layer_tile.m_gid);
                    if (!tile ||
                        tile->m_collision_rect.m_half_size == Vec2::ZERO) {
                        continue;
                    }

                    Rect rect;
                    rect.m_half_size = tile->m_collision_rect.m_half_size;
                    float scale =
                        std::max(transform->m_scale.x, transform->m_scale.y);
                    rect.m_center = tile->m_collision_rect.m_center * scale;

                    auto flip = layer_tile.m_flip;
                    if (flip & Flip::Vertical) {
                        float offset_y =  game_config.m_tile_size_h * 0.5 - rect.m_center.y;
                        rect.m_center.y += offset_y * 2.0;
                    }
                    if (flip & Flip::Horizontal) {
                        float offset_x = game_config.m_tile_size_w * 0.5 - rect.m_center.x;
                        rect.m_center.x += offset_x * 2.0;
                    }

                    rect.m_center +=
                        transform->m_position +
                        Vec2(x, y) * m_handle->GetTileSize() * scale;
                    rect.m_half_size *= scale;

                    physics_scene->CreateActorInChunk(rect.m_center, rect);
                }
            }
        }
    }
}

void TilemapComponentManager::Update() {
    for (auto& [entity, tilemap] : m_components) {
        if (!tilemap.m_enable) {
            continue;
        }

        auto transform = GAME_CONTEXT.m_transform_manager->Get(entity);
        if (!transform) {
            continue;
        }

        drawTilemap(*transform, tilemap.m_component->GetHandle());
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
                    Rect dst_rect;
                    dst_rect.m_half_size = tile->m_region.m_size * 0.5;

                    float scale =
                        std::max(transform.m_scale.x, transform.m_scale.y);

                    Vec2 scaled_tile_size = tilemap->GetTileSize() * scale;

                    dst_rect.m_center = transform.m_position +
                                        Vec2(x, y) * scaled_tile_size + scaled_tile_size * 0.5;

                    constexpr float scale_expand = 0.01;
                    scale += scale_expand;
                    dst_rect.m_half_size *= scale;

                    Region dst_region;
                    dst_region.m_topleft =
                        dst_rect.m_center - dst_rect.m_half_size;
                    dst_region.m_size = dst_rect.m_half_size * 2.0;

                    renderer->DrawImage(*tile->m_image, tile->m_region,
                                        dst_region, 0, {0, 0},
                                        layer_tile.m_flip);
                }
            }
        }
    }
}
