#include "tilemap.hpp"
#include "context.hpp"
#include "log.hpp"
#include "macro.hpp"


namespace tl {

TileSet::operator bool() const {
    return texture && tileSize != Vec2{0, 0};
}

const ObjectLayer* MapLayer::AsObjectLayer() const {
    return static_cast<const ObjectLayer*>(this);
}

const TileLayer* MapLayer::AsTileLayer() const {
    return static_cast<const TileLayer*>(this);
}

void Tile::UpdateTransform(const Transform& parentTrans, const Vec2& grid,
                           const Vec2& tileSize) {
    Transform localTransform;
    localTransform.position = grid * tileSize;
    globalTransform_ = CalcTransformFromParent(parentTrans, localTransform);
}

const Transform& Tile::GetGlobalTransform() const {
    return globalTransform_;
}

TileLayer::TileLayer(uint32_t w, uint32_t h)
    : MapLayer{MapLayerType::Tiles}, w_{w}, h_{h} {
    tiles_.resize(w * h);
}

void TileLayer::SetTile(uint32_t x, uint32_t y, const Tile& tile) {
    uint32_t idx = x + y * w_;
    TL_RETURN_IF_FALSE(idx < tiles_.size());

    tiles_[idx] = tile;
}

const Tile* TileLayer::GetTile(uint32_t x, uint32_t y) const {
    uint32_t idx = x + y * w_;
    TL_RETURN_NULL_IF_FALSE(idx < tiles_.size());

    return &tiles_[idx];
}

Tile* TileLayer::GetTile(uint32_t x, uint32_t y) {
    return const_cast<Tile*>(std::as_const(*this).GetTile(x, y));
}

Vec2 TileLayer::GetSize() const {
    return Vec2(w_, h_);
}

void TileLayer::UpdateTransform(const Transform& parentTrans,
                                const TileMap* tilemap) {
    for (uint32_t y = 0; y < h_; y++) {
        for (uint32_t x = 0; x < w_; x++) {
            auto tile = GetTile(x, y);
            TL_CONTINUE_IF_FALSE(tile->tilesetIndex);
            auto& tileset = tilemap->GetTileSet(tile->tilesetIndex.value());
            tile->UpdateTransform(parentTrans, Vec2(x, y), tileset.tileSize);
        }
    }
}

TileMap::TileMap(const std::string& filename) {
    size_t fileSize;
    void* fileContent = SDL_LoadFile(filename.c_str(), &fileSize);
    TL_RETURN_IF_FALSE_LOGE(fileContent, "%s load failed", filename.c_str());

    tson::Tileson t;
    std::unique_ptr<tson::Map> map = t.parse(fileContent, fileSize);

    TL_RETURN_IF_FALSE_LOGW(map->getStatus() == tson::ParseStatus::OK,
                            "%s tilemap parse failed", filename.c_str());

    TL_RETURN_IF_FALSE_LOGE(!map->isInfinite(),
                            "only support finite tilemap(from %s)",
                            filename.c_str());

    for (auto& tileset : map->getTilesets()) {
        tileSets_.emplace_back(parseTileSet(tileset));
    }

    auto properties = map->getProperties();

    for (auto& layer : map->getLayers()) {
        if (layer.getType() == tson::LayerType::ObjectGroup) {
            auto result = parseObjectLayer(layer);
            if (result) {
                layers_.emplace_back(std::move(result));
            }
        } else if (layer.getType() == tson::LayerType::TileLayer) {
            auto result = parseTileLayer(layer);
            if (result) {
                layers_.emplace_back(std::move(result));
            }
        } else {
            // TODO: parse other layer type
        }
    }
}

void TileMap::UpdateTransform(const Transform& parentTrans) {
    for (auto& layer : layers_) {
        if (layer->GetType() == MapLayerType::Tiles) {
            layer->AsTileLayer()->UpdateTransform(parentTrans, this);
        }
    }
}

std::unique_ptr<ObjectLayer> TileMap::parseObjectLayer(
    const tson::Layer& layer) const {
    auto result = std::make_unique<ObjectLayer>();

    auto& objects = layer.getObjects();
    for (auto& object : objects) {
        if (object.getObjectType() == tson::ObjectType::Ellipse) {
            Ellipse ellipse;
            ellipse.center.x = object.getPosition().x;
            ellipse.center.y = object.getPosition().y;
            ellipse.halfX = object.getSize().x * 0.5;
            ellipse.halfY = object.getSize().y * 0.5;
            ellipse.properties = &object.getProperties();
            result->ellipses.emplace_back(std::move(ellipse));
        } else if (object.getObjectType() == tson::ObjectType::Point) {
            Point point;
            point.x = object.getPosition().x;
            point.y = object.getPosition().y;
            point.properties = &object.getProperties();
            result->points.emplace_back(std::move(point));
        } else if (object.getObjectType() == tson::ObjectType::Rectangle) {
            Rect rect;
            rect.position.x = object.getPosition().x;
            rect.position.y = object.getPosition().y;
            rect.size.w = object.getSize().x;
            rect.size.h = object.getSize().y;
            rect.properties = &object.getProperties();
            result->rects.emplace_back(std::move(rect));
        } else if (object.getObjectType() == tson::ObjectType::Polygon) {
            Polygon polygon;
            for (auto& p : object.getPolygons()) {
                Vec2 point;
                point.x = p.x;
                point.y = p.y;
                polygon.points.emplace_back(std::move(point));
            }
            polygon.properties = &object.getProperties();
            result->polygons.emplace_back(std::move(polygon));
        } else if (object.getObjectType() == tson::ObjectType::Polyline) {
            Polyline polyline;
            for (auto& p : object.getPolylines()) {
                Vec2 point;
                point.x = p.x;
                point.y = p.y;
                polyline.points.emplace_back(std::move(point));
            }
            polyline.properties = &object.getProperties();
            result->polylines.emplace_back(std::move(polyline));
        } else if (object.getObjectType() == tson::ObjectType::Object) {
            ObjectLayer::TileObject obj;
            obj.size.w = object.getSize().x;
            obj.size.h = object.getSize().y;
            obj.position.w = object.getPosition().x;
            obj.position.h = object.getPosition().y;
            auto tileset = layer.getMap()->getTilesetByGid(object.getGid());
            const tson::Tile* tile = tileset->getTile(object.getGid());
            obj.region.size.w = tile->getDrawingRect().width;
            obj.region.size.h = tile->getDrawingRect().height;
            obj.region.position.x = tile->getDrawingRect().x;
            obj.region.position.y = tile->getDrawingRect().y;
            if (object.getFlipFlags() == tson::TileFlipFlags::Vertically) {
                obj.flip = Flip::Vertical;
            }
            if (object.getFlipFlags() == tson::TileFlipFlags::Horizontally) {
                obj.flip = Flip::Horizontal;
            }
            if (object.getFlipFlags() == tson::TileFlipFlags::Diagonally) {
                obj.flip = Flip::Horizontal;
                obj.flip |= Flip::Vertical;
            }
            for (auto& ts : tileSets_) {
                if (ts.name == tileset->getName()) {
                    obj.tileset = &ts;
                    break;
                }
            }
            obj.properties = &object.getProperties();
            result->tileObjects.emplace_back(std::move(obj));
        }
    }

    return result;
}

std::unique_ptr<TileLayer> TileMap::parseTileLayer(
    const tson::Layer& layer) const {
    auto mapSize = layer.getMap()->getSize();
    auto tileLayer = std::make_unique<TileLayer>(mapSize.x, mapSize.y);

    Flags<tson::TileFlipFlags> flipFlags;
    flipFlags |= tson::TileFlipFlags::Diagonally;
    flipFlags |= tson::TileFlipFlags::Horizontally;
    flipFlags |= tson::TileFlipFlags::Vertically;

    for (auto& [pos, tileObj] : layer.getTileObjects()) {
        auto [x, y] = pos;

        Tile myTile;

        const tson::Rect& rect = tileObj.getDrawingRect();
        myTile.region.position.x = rect.x;
        myTile.region.position.y = rect.y;
        myTile.region.size.w = rect.width;
        myTile.region.size.h = rect.height;

        auto tile = tileObj.getTile();
        auto flip = tile->getFlipFlags();
        if (flip == tson::TileFlipFlags::Vertically) {
            myTile.flip = Flip::Vertical;
        } else if (flip == tson::TileFlipFlags::Horizontally) {
            myTile.flip = Flip::Horizontal;
        } else if (flip == tson::TileFlipFlags::Diagonally) {
            myTile.flip = Flip::Horizontal;
            myTile.flip |= Flip::Vertical;
        }

        auto& tilesetName = tileObj.getTile()->getTileset()->getName();
        for (int i = 0; i < tileSets_.size(); i++) {
            if (tileSets_[i].name == tilesetName) {
                myTile.tilesetIndex = i;
                break;
            }
        }

        auto nameProp = tile->getProp("name");
        if (nameProp) {
            myTile.name = nameProp->getValue<std::string>();
        }

        // find collision shape
        uint32_t id = tile->getId() & static_cast<uint32_t>(~flipFlags);
        if (auto it = collisionMap_.find(id); it != collisionMap_.end()) {
            if (it->second.shape.type== Shape::Type::Circle) {
                const Circle& c = it->second.shape.circle;
                myTile.actor.enable = true;
                myTile.actor.shape.type = Shape::Type::Circle;
                myTile.actor.shape.circle = c;
                myTile.actor.isTrigger = it->second.isTrigger;
                myTile.actor.filter = it->second.collisionGroup;
            } else if (it->second.shape.type== Shape::Type::AABB) {
                const AABB& a = it->second.shape.aabb;
                myTile.actor.enable = true;
                myTile.actor.shape.type = Shape::Type::AABB;
                myTile.actor.shape.aabb = a;
                myTile.actor.isTrigger = it->second.isTrigger;
                myTile.actor.filter = it->second.collisionGroup;
            }
        }

        tileLayer->SetTile(x, y, myTile);
    }

    return tileLayer;
}

TileSet TileMap::parseTileSet(const tson::Tileset& tileset) {
    TileSet result;
    auto& imageName = tileset.getImage();
    Texture* texture = Context::GetInst().textureMgr->Find(imageName);

    if (!texture) {
        LOGW("%s image not exists when parsing tileset %s", imageName.c_str(),
             tileset.getName().c_str());
        return {};
    }

    Flags<tson::TileFlipFlags> flipFlags;
    flipFlags |= tson::TileFlipFlags::Diagonally;
    flipFlags |= tson::TileFlipFlags::Horizontally;
    flipFlags |= tson::TileFlipFlags::Vertically;

    auto& tiles = tileset.getTiles();
    for (auto& tile : tiles) {
        auto gid = tile.getId();
        uint32_t id = gid & static_cast<uint32_t>(~flipFlags);
        auto layer = parseObjectLayer(tile.getObjectgroup());
        // NOTE: we only use one collision body in layer, and we only support
        // Circle & AABB
        TileMapCollision collision;
        if (!layer->ellipses.empty()) {
            const Ellipse& ellipse = layer->ellipses[0];
            TL_CONTINUE_IF_FALSE(ellipse);
            collision.shape.SetCircle(Circle{ellipse.center, ellipse.halfX});
            auto triggerProperty = ellipse.properties->getProperty("trigger");
            if (triggerProperty) {
                collision.isTrigger = triggerProperty->getValue<bool>();
            }
            if (ellipse.properties->hasProperty("collision_group")) {
                auto collisionGroupProperty = ellipse.properties->getProperty("collision_group");
                collision.collisionGroup = collisionGroupProperty->getValue<int>();
            }
            collisionMap_[id] = collision;
        } else if (!layer->rects.empty()) {
            const Rect& rect = layer->rects[0];
            TL_CONTINUE_IF_FALSE(rect);
            if (rect.properties->hasProperty("trigger")) {
                auto property = rect.properties->getProperty("trigger");
                collision.isTrigger = property->getValue<bool>();
            }
            if (rect.properties->hasProperty("collision_group")) {
                auto collisionGroupProperty = rect.properties->getProperty("collision_group");
                collision.collisionGroup = collisionGroupProperty->getValue<int>();
            }
            collision.shape.SetAABB(
                AABB{rect.position + rect.size * 0.5, rect.size * 0.5});
            collisionMap_[id] = collision;
        }
    }

    result.name = tileset.getName();
    result.texture = texture;
    result.margin = tileset.getMargin();
    result.padding = tileset.getSpacing();
    result.tileSize.w = tileset.getTileSize().x;
    result.tileSize.h = tileset.getTileSize().y;

    return result;
}

TileMap* TileMapManager::Load(const std::string& filename,
                              const std::string& name) {
    auto it = tilemaps_.find(name);
    if (it != tilemaps_.end()) {
        LOGW("%s tilemap already exists", name.c_str());
        return &it->second;
    }

    return &tilemaps_.emplace(name, filename).first->second;
}

TileMap* TileMapManager::Find(const std::string& name) {
    auto it = tilemaps_.find(name);
    if (it != tilemaps_.end()) {
        return &it->second;
    }
    return nullptr;
}


}  // namespace tl
