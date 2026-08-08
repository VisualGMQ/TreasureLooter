#pragma once
#include "common/tilemap_layer_collision_component.hpp"

#include <unordered_set>

class ClientTilemapLayerCollisionComponentManager
    : public TilemapLayerCollisionComponentManager {
public:
    void RenderDebug();
    void EnableDebugEntity(Entity, bool);

private:
    std::unordered_set<Entity> m_debug_entities;

    void renderCollision(TilemapLayerCollisionComponent&, IDebugDrawer&);
};