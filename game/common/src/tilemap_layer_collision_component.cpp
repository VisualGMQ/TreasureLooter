#include "common/tilemap_layer_collision_component.hpp"

TilemapLayerCollisionComponent::TilemapLayerCollisionComponent(
    Entity entity, const TilemapLayerDefinition& create_info) {
    TL_RETURN_IF_FALSE(create_info.m_tilemap);

    m_tilemap_handle = create_info.m_tilemap;

    for (auto& layer : create_info.m_tilemap->GetLayers()) {
        if (layer->GetName() == create_info.m_layer_name) {
            switch (layer->GetType()) {
                case TilemapLayer::Type::Tiled:
                    m_tilemap_layer = std::make_unique<TilemapTileLayer>(
                        *layer->AsTiledLayer());
                    break;
                case TilemapLayer::Type::Object:
                    m_tilemap_layer = std::make_unique<TilemapObjectLayer>(
                        *layer->AsObjectLayer());
                    break;
                case TilemapLayer::Type::Image:
                    m_tilemap_layer = std::make_unique<TilemapImageLayer>(
                        *layer->AsImageLayer());
                    break;
            }
        }
    }

    TL_RETURN_IF_FALSE_WITH_LOG(
        m_tilemap_layer, LOGE,
        "[Tilemap]: create tilemap layer {} from tilemap {} failed",
        create_info.m_layer_name,
        create_info.m_tilemap.GetFilename()->string());

    auto& game_config = COMMON_CONTEXT.GetGameConfig();
    auto& physics_scene = COMMON_CONTEXT.m_physics_scene;

    auto tilemap = create_info.m_tilemap;

    auto tile_size = tilemap->GetTileSize();
    m_tilemap_collision = PhysicsScene::TilemapCollision::Proxy{
        physics_scene->CreateTilemapCollision(create_info.m_position)};

    if (m_tilemap_layer->GetType() == TilemapLayer::Type::Tiled) {
        auto tiled_layer = m_tilemap_layer->AsTiledLayer();
        m_tilemap_collision->CreateLayer(Vec2UI(tile_size.w, tile_size.h),
                                         game_config.m_tile_in_chunk_size);
        auto& size = tiled_layer->GetSize();
        for (size_t y = 0; y < size.y; y++) {
            for (size_t x = 0; x < size.x; x++) {
                auto& layer_tile = tiled_layer->GetTile(x, y);
                auto tile = tilemap->GetTile(layer_tile.m_gid);
                if (!tile || tile->m_collision_rect.m_half_size == Vec2::ZERO) {
                    continue;
                }

                Rect rect = tile->m_collision_rect;

                auto flip = layer_tile.m_flip;
                if (flip & Flip::Vertical) {
                    float offset_y =
                        tile->m_tile_size.h * 0.5 - rect.m_center.y;
                    rect.m_center.y += offset_y * 2.0;
                }
                if (flip & Flip::Horizontal) {
                    float offset_x =
                        tile->m_tile_size.w * 0.5 - rect.m_center.x;
                    rect.m_center.x += offset_x * 2.0;
                }

                rect.m_center += create_info.m_position +
                                 Vec2(x, y + 1) * tilemap->GetTileSize() +
                                 Vec2(0, -tile->m_tile_size.h);

                PhysicsShapeDefinition definition;
                definition.m_is_rect = true;
                definition.m_rect = rect;
                definition.m_collision_layer.Add(CollisionGroupType::Obstacle);
                definition.m_collision_mask.Add(CollisionGroupType::CCT);

                physics_scene->CreateShapeInChunk(
                    entity, m_tilemap_collision.get(), definition);
            }
        }
    }
}

const TilemapLayer* TilemapLayerCollisionComponent::GetLayer() const {
    return m_tilemap_layer.get();
}

const Tilemap* TilemapLayerCollisionComponent::GetTilemap() const {
    return m_tilemap_handle.Get();
}

const PhysicsScene::TilemapCollision*
TilemapLayerCollisionComponent::GetTilemapCollision() const {
    return m_tilemap_collision.get();
}
