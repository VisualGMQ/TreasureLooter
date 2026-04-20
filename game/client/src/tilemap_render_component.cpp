#include "client/tilemap_render_component.hpp"
#include "client/context.hpp"
#include "client/draw_order.hpp"
#include "client/image.hpp"
#include "client/renderer.hpp"
#include "common/profile.hpp"

TilemapLayerRenderComponent::TilemapLayerRenderComponent(
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

    m_tilemap_collision = PhysicsScene::TilemapCollision::Proxy{
        COMMON_CONTEXT.m_physics_scene->CreateTilemapCollision(
            create_info.m_position)};
}

const TilemapLayer* TilemapLayerRenderComponent::GetLayer() const {
    return m_tilemap_layer.get();
}

const Tilemap* TilemapLayerRenderComponent::GetTilemap() const {
    return m_tilemap_handle.Get();
}

const PhysicsScene::TilemapCollision*
TilemapLayerRenderComponent::GetTilemapCollision() const {
    return m_tilemap_collision.get();
}

void TilemapLayerRenderComponentManager::SubmitDrawCommand(Entity entity) {
    PROFILE_SECTION();

    auto tilemap_layer = Get(entity);
    TL_RETURN_IF_FALSE(IsEnable(entity) && tilemap_layer->GetLayer() &&
                       tilemap_layer->GetTilemap());

    drawTilemapLayer(CLIENT_CONTEXT.m_draw_order_manager->Get(entity),
                     *tilemap_layer);
}

void TilemapLayerRenderComponentManager::drawTilemapLayer(
    const DrawOrder* draw_order, const TilemapLayerRenderComponent& component) {
    auto& renderer = CLIENT_CONTEXT.m_renderer;
    auto tilemap_layer = component.GetLayer();
    const Tilemap* tilemap = component.GetTilemap();
    if (tilemap_layer->GetType() == TilemapLayer::Type::Tiled) {
        auto tiled_layer = tilemap_layer->AsTiledLayer();
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

                float scale = 1.0;
                Vec2 scaled_tile_size = tilemap->GetTileSize() * scale;

                dst_rect.m_center =
                    component.GetTilemapCollision()->m_topleft +
                    Vec2(x, y + 1) * scaled_tile_size +
                    Vec2(tile->m_tile_size.w, -tile->m_tile_size.h) * 0.5;

                constexpr float scale_expand = 0.01;
                scale += scale_expand;
                dst_rect.m_half_size *= scale;

                Region dst_region;
                dst_region.m_topleft = dst_rect.m_center - dst_rect.m_half_size;
                dst_region.m_size = dst_rect.m_half_size * 2.0;

                renderer->DrawImage(
                    *tile->m_image, tile->m_region, dst_region, Color::White, 0,
                    {0, 0}, layer_tile.m_flip, draw_order->GetGlobalOrder(),
                    true, dst_rect.m_center.y + dst_rect.m_half_size.y);
            }
        }
    } else if (tilemap_layer->GetType() == TilemapLayer::Type::Image) {
        auto image_layer = tilemap_layer->AsImageLayer();
        ImageHandle image = image_layer->GetImage();
        if (image) {
            renderer->DrawImage(
                *image, Region{Vec2::ZERO, image->GetSize()},
                Region{image_layer->GetPosition(), image->GetSize()},
                Color::White, 0, Vec2::ZERO, Flip::None,
                draw_order->GetGlobalOrder(), true, 0);
        }
    }
}
