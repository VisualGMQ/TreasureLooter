#include "client/tilemap_layer_collision_component.hpp"

#include "client/context.hpp"
#include "common/debug_drawer.hpp"

void ClientTilemapLayerCollisionComponentManager::RenderDebug() {
    for (Entity entity : m_debug_entities) {
        auto collision = Get(entity);
        renderCollision(*collision, *CLIENT_CONTEXT.m_debug_drawer);
    }
}

void ClientTilemapLayerCollisionComponentManager::EnableDebugEntity(
    Entity entity, bool enable) {
    if (enable) {
        m_debug_entities.insert(entity);
    } else {
        m_debug_entities.erase(entity);
    }
}

void ClientTilemapLayerCollisionComponentManager::renderCollision(
    TilemapLayerCollisionComponent& collision, IDebugDrawer& drawer) {
    auto tilemap_collision = collision.GetTilemapCollision();
    Vec2 tile_size = collision.GetTilemap()->GetTileSize();
    auto& chunks = tilemap_collision->m_chunks;
    Color color = Color::Green;
    color.a = 0.5;
    Rect rect;
    for (int row1 = 0; row1 < chunks.m_chunks.GetHeight(); row1++) {
        for (int col1 = 0; col1 < chunks.m_chunks.GetWidth(); col1++) {
            auto& chunk = chunks.m_chunks.Get(col1, row1);
            for (int row2 = 0; row2 < chunk.GetHeight(); row2++) {
                for (int col2 = 0; col2 < chunk.GetWidth(); col2++) {
                    TL_CONTINUE_IF_FALSE(!chunk.Get(col2, row2).empty());

                    rect.m_half_size = tile_size * 0.5f;
                    auto chunk_w = chunk.GetWidth();
                    auto chunk_h = chunk.GetHeight();
                    rect.m_center.x =
                        tile_size.w * (col1 * chunk_w + col2 + 0.5);
                    rect.m_center.y =
                        tile_size.h * (row1 * chunk_h + row2 + 0.5);
                    drawer.FillRect(rect, color, IDebugDrawer::kOneFrame, true);
                }
            }
        }
    }
}