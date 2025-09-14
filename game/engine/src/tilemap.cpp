#include "engine/tilemap.hpp"

#include "engine/context.hpp"
#include "engine/asset_manager.hpp"
#include "engine/log.hpp"
#include "schema/tilemap_schema.hpp"
#include "tmxlite/TileLayer.hpp"

// Bits on the far end of the 32-bit global tile ID are used for tile flags
const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x8;
const unsigned FLIPPED_VERTICALLY_FLAG = 0x4;
const unsigned FLIPPED_DIAGONALLY_FLAG = 0x2;
const unsigned ROTATED_HEXAGONAL_120_FLAG = 0x1;

TilemapLayer::TilemapLayer(Type type) : m_type{type} {
}

const TilemapTileLayer *TilemapLayer::CastAsTiledLayer() const {
    if (m_type == Type::Tiled) {
        return static_cast<const TilemapTileLayer *>(this);
    }
    return nullptr;
}

TilemapLayer::Type TilemapLayer::GetType() const {
    return m_type;
}

TilemapTileLayer::TilemapTileLayer(const tmx::TileLayer &layer)
    : TilemapLayer(Type::Tiled) {
    parse(layer);
}

const TilemapTileLayer::Tile &TilemapTileLayer::GetTile(int x, int y) const {
    size_t idx = y * m_size.x + x;
    return m_tiles[idx];
}

const Vec2 &TilemapTileLayer::GetSize() const {
    return m_size;
}

void TilemapTileLayer::parse(const tmx::TileLayer &layer) {
    auto size = layer.getSize();
    m_size.x = size.x;
    m_size.y = size.y;
    for (int y = 0; y < size.y; y++) {
        for (int x = 0; x < size.x; x++) {
            const auto idx = y * size.x + x;
            auto &tmx_tile = layer.getTiles()[idx];
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

Tileset::Tileset(const tmx::Tileset &tileset) {
    tileset.getTileSize();
    parse(tileset);
}

void Tileset::parse(const tmx::Tileset &tileset) {
    auto path = tileset.getImagePath();
    m_image = CURRENT_CONTEXT.m_assets_manager->GetManager<Image>().Load(path);
    m_margin = tileset.getMargin();
    m_spacing = tileset.getSpacing();
    m_firstgid = tileset.getFirstGID();
    m_lastgid = tileset.getLastGID();
    m_tile_size.w = tileset.getTileSize().x;
    m_tile_size.h = tileset.getTileSize().y;

    for (uint32_t i = 0; i < tileset.getTileCount(); i++) {
        auto tmx_tile = tileset.getTile(i + tileset.getFirstGID());
        Tile tile;
        tile.m_id = tmx_tile->ID;
        tile.m_image = m_image;
        tile.m_region.m_topleft.x = tmx_tile->imagePosition.x;
        tile.m_region.m_topleft.y = tmx_tile->imagePosition.y;
        tile.m_region.m_size.x = tmx_tile->imageSize.x;
        tile.m_region.m_size.y = tmx_tile->imageSize.y;
        tile.m_tile_size.x = tileset.getTileSize().x;
        tile.m_tile_size.y = tileset.getTileSize().y;

        auto &objs = tmx_tile->objectGroup.getObjects();
        if (!objs.empty()) {
            auto &obj = objs[0];
            if (obj.getShape() == tmx::Object::Shape::Rectangle) {
                auto &aabb = obj.getAABB();
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

const Tile &Tileset::GetTile(uint32_t gid) const {
    auto idx = gid - m_firstgid;
    return m_tiles[idx];
}

bool Tileset::HasTile(uint32_t gid) const {
    return gid >= m_firstgid && gid < m_lastgid;
}

const Vec2 &Tileset::GetTileSize() const { return m_tile_size; }

Tilemap::Tilemap(const Path &filename) : m_filename{filename} {
    parse(filename);
}

const Tile *Tilemap::GetTile(uint32_t gid) const {
    for (auto &tileset: m_tilesets) {
        if (tileset.HasTile(gid)) {
            return &tileset.GetTile(gid);
        }
    }
    return nullptr;
}

const Vec2 &Tilemap::GetTileSize() const {
    return m_tile_size;
}

const Path &Tilemap::GetFilename() const {
    return m_filename;
}

void Tilemap::parse(const Path &filename) {
    tmx::Map map;
    if (!map.load(filename.string())) {
        LOGE("load tilemap {} failed", filename);
        return;
    }

    m_tile_size.w = map.getTileSize().x;
    m_tile_size.h = map.getTileSize().y;

    auto &layers = map.getLayers();
    for (auto &layer: layers) {
        if (layer->getType() == tmx::Layer::Type::Tile) {
            const auto &tile_layer = layer->getLayerAs<tmx::TileLayer>();
            m_layers.emplace_back(
                std::make_unique<TilemapTileLayer>(tile_layer));
        }
    }

    const auto &tilesets = map.getTilesets();
    for (const auto &tileset: tilesets) {
        m_tilesets.emplace_back(tileset);
    }
}

TilemapHandle TilemapManager::Load(const Path &filename, bool force) {
    if (auto handle = Find(filename); handle && !force) {
        return handle;
    }
    return store(&filename, UUIDv4::Create(),
                 std::make_unique<Tilemap>(filename));
}

TilemapComponent::TilemapComponent(Entity entity,
                                   const TilemapInfo &create_info)
    : m_handle{create_info.m_tilemap} {
    if (!m_handle) {
        return;
    }

    auto &game_config = CURRENT_CONTEXT.GetGameConfig();
    auto &physics_scene = CURRENT_CONTEXT.m_physics_scene;

    auto tile_size = m_handle->GetTileSize();
    m_tilemap_collision = physics_scene->CreateTilemapCollision(create_info.m_position);

    auto &layers = m_handle->GetLayers();
    for (size_t i = 0; i < layers.size(); i++) {
        auto &layer = layers[i];
        if (layer->GetType() == TilemapLayer::Type::Tiled) {
            auto tiled_layer = layer->CastAsTiledLayer();
            m_tilemap_collision->CreateLayer(
                Vec2UI(tile_size.w, tile_size.h), game_config.m_tile_in_chunk_size);
            auto &size = tiled_layer->GetSize();
            for (size_t y = 0; y < size.y; y++) {
                for (size_t x = 0; x < size.x; x++) {
                    auto &layer_tile = tiled_layer->GetTile(x, y);
                    auto tile = m_handle->GetTile(layer_tile.m_gid);
                    if (!tile ||
                        tile->m_collision_rect.m_half_size == Vec2::ZERO) {
                        continue;
                    }

                    Rect rect = tile->m_collision_rect;

                    auto flip = layer_tile.m_flip;
                    if (flip & Flip::Vertical) {
                        float offset_y = tile->m_tile_size.h * 0.5 - rect.m_center.y;
                        rect.m_center.y += offset_y * 2.0;
                    }
                    if (flip & Flip::Horizontal) {
                        float offset_x = tile->m_tile_size.w * 0.5 - rect.m_center.x;
                        rect.m_center.x += offset_x * 2.0;
                    }

                    rect.m_center +=
                            create_info.m_position +
                            Vec2(x, y + 1) * m_handle->GetTileSize() + Vec2(0, -tile->m_tile_size.h);

                    PhysicsShape shape{rect};
                    auto actor = physics_scene->CreateActorInChunk(
                        entity, m_tilemap_collision, i, shape);
                    CollisionGroup collision_layer;
                    collision_layer.Add(CollisionGroupType::Obstacle);
                    actor->SetCollisionLayer(collision_layer);
                    CollisionGroup collision_mask;
                    collision_mask.Add(CollisionGroupType::CCT);
                    actor->SetCollisionMask(collision_mask);
                }
            }
        }
    }
}

TilemapHandle TilemapComponent::GetHandle() const { return m_handle; }

const PhysicsScene::TilemapCollision *TilemapComponent::GetTilemapCollision() const {
    return m_tilemap_collision;
}

void TilemapComponentManager::Update() {
    for (auto &[entity, tilemap]: m_components) {
        if (!tilemap.m_enable) {
            continue;
        }

        drawTilemap(*tilemap.m_component);
    }
}

void TilemapComponentManager::drawTilemap(const TilemapComponent &tilemap) {
    auto &renderer = CURRENT_CONTEXT.m_renderer;
    auto handle = tilemap.GetHandle();
    for (auto &layer: handle->GetLayers()) {
        if (layer->GetType() == TilemapLayer::Type::Tiled) {
            auto tiled_layer = layer->CastAsTiledLayer();
            auto &size = tiled_layer->GetSize();
            for (size_t y = 0; y < size.y; y++) {
                for (size_t x = 0; x < size.x; x++) {
                    auto &layer_tile = tiled_layer->GetTile(x, y);
                    auto tile = handle->GetTile(layer_tile.m_gid);
                    if (!tile) {
                        continue;
                    }
                    Rect dst_rect;
                    dst_rect.m_half_size = tile->m_region.m_size * 0.5;

                    float scale = 1.0;
                    Vec2 scaled_tile_size = handle->GetTileSize() * scale;

                    dst_rect.m_center = tilemap.GetTilemapCollision()->m_topleft +
                                        Vec2(x, y + 1) * scaled_tile_size + Vec2(
                                            tile->m_tile_size.w, -tile->m_tile_size.h) * 0.5;

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
