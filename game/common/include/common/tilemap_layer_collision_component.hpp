#pragma once

#include "common/context.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/tilemap.hpp"
#include "schema/tilemap_schema.hpp"

class TilemapLayerCollisionComponent {
public:
    TilemapLayerCollisionComponent(Entity entity,
                                   const TilemapLayerDefinition& create_info);

    [[nodiscard]] const TilemapLayer* GetLayer() const;
    [[nodiscard]] const Tilemap* GetTilemap() const;

    const PhysicsScene::TilemapCollision* GetTilemapCollision() const;

private:
    std::unique_ptr<TilemapLayer> m_tilemap_layer;
    TilemapHandle m_tilemap_handle;  // FIXME: component rely on asset may cause
                                     // asset dangling reference
    std::string m_name;
    PhysicsScene::TilemapCollision::Proxy m_tilemap_collision{};
};

class TilemapLayerCollisionComponentManager
    : public ComponentManager<TilemapLayerCollisionComponent> {};
