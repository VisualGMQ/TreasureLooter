#pragma once
#include "common/tilemap_layer_collision_component.hpp"

#include <unordered_set>

class ClientTilemapLayerCollisionComponentManager
    : public TilemapLayerCollisionComponentManager {
public:
    void RenderDebug();
    void EnableDebugEntity(LogicEntity, bool);

private:
    std::unordered_set<LogicEntity> m_debug_entities;

    void renderCollision(TilemapLayerCollisionComponent&, IDebugDrawer&);
};