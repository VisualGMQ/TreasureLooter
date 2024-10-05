#include "tilemap.hpp"
#include "context.hpp"
#include "log.hpp"
#include "macro.hpp"


namespace tl {

TileSet::operator bool() const {
    return texture && tileSize != Vec2{0, 0};
}

TileLayer::TileLayer(uint32_t w, uint32_t h)
    : MapLayer{MapLayerType::Tiles}, w_{w}, h_{h} {
    tiles_.resize(w * h);
}

void TileLayer::SetTile(uint32_t x, uint32_t y, const Tile& tile) {
    uint32_t idx = x + y * w_;
    TL_RETURN_IF(idx < tiles_.size());

    tiles_[idx] = tile;
}

const Tile* TileLayer::GetTile(uint32_t x, uint32_t y) const {
    uint32_t idx = x + y * w_;
    TL_RETURN_NULL_IF(idx < tiles_.size());

    return &tiles_[idx];
}

Vec2 TileLayer::GetSize() const {
    return Vec2(w_, h_);
}

TileMap::TileMap(const std::string& filename) {
    tson::Tileson t;
    std::unique_ptr<tson::Map> map = t.parse(filename);

    TL_RETURN_IF_LOGW(map->getStatus() == tson::ParseStatus::OK,
                      "%s tilemap parse failed", filename.c_str());

    TL_RETURN_IF_LOGE(!map->isInfinite(),
                      "only support finite tilemap(from %s)", filename.c_str());

    for (auto& tileset : map->getTilesets()) {
        tileSets_.emplace_back(parseTileSet(tileset));
    }

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
            result->ellipses.emplace_back(std::move(ellipse));
        } else if (object.getObjectType() == tson::ObjectType::Point) {
            Vec2 point;
            point.x = object.getPosition().x;
            point.y = object.getPosition().y;
            result->points.emplace_back(std::move(point));
        } else if (object.getObjectType() == tson::ObjectType::Rectangle) {
            Rect rect;
            rect.position.x = object.getPosition().x;
            rect.position.y = object.getPosition().y;
            rect.size.w = object.getSize().x;
            rect.size.h = object.getSize().y;
            result->rects.emplace_back(std::move(rect));
        } else if (object.getObjectType() == tson::ObjectType::Polygon) {
            Polygon polygon;
            for (auto& p : object.getPolygons()) {
                Vec2 point;
                point.x = p.x;
                point.y = p.y;
                polygon.points.emplace_back(std::move(point));
            }
            result->polygons.emplace_back(std::move(polygon));
        } else if (object.getObjectType() == tson::ObjectType::Polyline) {
            Polyline polyline;
            for (auto& p : object.getPolylines()) {
                Vec2 point;
                point.x = p.x;
                point.y = p.y;
                polyline.points.emplace_back(std::move(point));
            }
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
            result->tileObjects.emplace_back(std::move(obj));
        }
    }

    return result;
}

std::unique_ptr<TileLayer> TileMap::parseTileLayer(
    const tson::Layer& layer) const {
    auto mapSize = layer.getMap()->getSize();
    auto tileLayer = std::make_unique<TileLayer>(mapSize.x, mapSize.y);

    auto& tileData = layer.getTileData();
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

        tileLayer->SetTile(x, y, myTile);
    }

    return tileLayer;
}

TileSet TileMap::parseTileSet(const tson::Tileset& tileset) const {
    TileSet result;
    auto& imageName = tileset.getImage();
    Texture* texture = Context::GetInst().textureMgr->Get(imageName);

    if (!texture) {
        LOGW("%s image not exists when parsing tileset %s", imageName.c_str(),
             tileset.getName().c_str());
        return {};
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
        return it->second.get();
    }

    return tilemaps_.emplace(name, std::make_unique<TileMap>(TileMap{filename}))
        .first->second.get();
}

TileMap* TileMapManager::Find(const std::string& name) {
    auto it = tilemaps_.find(name);
    if (it != tilemaps_.end()) {
        return it->second.get();
    }
    return nullptr;
}

}  // namespace tl