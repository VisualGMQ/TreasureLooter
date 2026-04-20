#include "common/tilemap.hpp"

#include "common/asset_manager.hpp"
#include "common/context.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/profile.hpp"
#include "schema/tilemap_schema.hpp"
#include "tmxlite/ObjectGroup.hpp"
#include "tmxlite/Property.hpp"
#include "tmxlite/TileLayer.hpp"
#include <variant>

// Bits on the far end of the 32-bit global tile ID are used for tile flags
constexpr unsigned FLIPPED_HORIZONTALLY_FLAG = 0x8;
constexpr unsigned FLIPPED_VERTICALLY_FLAG = 0x4;
constexpr unsigned FLIPPED_DIAGONALLY_FLAG = 0x2;
constexpr unsigned ROTATED_HEXAGONAL_120_FLAG = 0x1;

constexpr std::string_view TilemapPropertyName_EnableYSort = "enable_y_sort";

TilemapLayer::TilemapLayer(const std::string& name, Type type)
    : m_type{type}, m_name{name} {}

const TilemapTileLayer* TilemapLayer::AsTiledLayer() const {
    TL_RETURN_DEFAULT_IF_FALSE(m_type == Type::Tiled);
    return static_cast<const TilemapTileLayer*>(this);
}

const class TilemapImageLayer* TilemapLayer::AsImageLayer() const {
    TL_RETURN_DEFAULT_IF_FALSE(m_type == Type::Image);
    return static_cast<const TilemapImageLayer*>(this);
}

const class TilemapObjectLayer* TilemapLayer::AsObjectLayer() const {
    TL_RETURN_DEFAULT_IF_FALSE(m_type == Type::Object);
    return static_cast<const TilemapObjectLayer*>(this);
}

TilemapLayer::Type TilemapLayer::GetType() const {
    return m_type;
}

TilemapTileLayer::TilemapTileLayer(const std::string& name,
                                   const tmx::TileLayer& layer)
    : TilemapLayer(name, Type::Tiled) {
    parse(layer);
}

const TilemapTileLayer::Tile& TilemapTileLayer::GetTile(int x, int y) const {
    size_t idx = y * m_size.x + x;
    return m_tiles[idx];
}

const Vec2& TilemapTileLayer::GetSize() const {
    return m_size;
}

const std::vector<TilemapProperty>& TilemapLayer::GetProperties() const {
    return m_properties;
}

std::string_view TilemapLayer::GetName() const {
    return m_name;
}

void TilemapTileLayer::parse(const tmx::TileLayer& layer) {
    // record Properties
    for (auto& prop : layer.getProperties()) {
        m_properties.emplace_back(TilemapProperty{prop});
    }

    // record tiles
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

TilemapProperty::TilemapProperty(int value) : m_data{value} {}

TilemapProperty::TilemapProperty(float value) : m_data{value} {}

TilemapProperty::TilemapProperty(bool value) : m_data{value} {}

TilemapProperty::TilemapProperty(const Color& value) : m_data{value} {}

TilemapProperty::TilemapProperty(const Path& value) : m_data{value} {}

TilemapProperty::TilemapProperty(const std::string& value) : m_data{value} {}

TilemapProperty::TilemapProperty(const tmx::Property& prop) {
    switch (prop.getType()) {
        case tmx::Property::Type::Boolean:
            *this = TilemapProperty{prop.getBoolValue()};
            break;
        case tmx::Property::Type::Float:
            *this = TilemapProperty{prop.getFloatValue()};
            break;
        case tmx::Property::Type::Int:
            *this = TilemapProperty{prop.getIntValue()};
            break;
        case tmx::Property::Type::String:
            *this = TilemapProperty{prop.getStringValue()};
            break;
        case tmx::Property::Type::Colour: {
            auto color = prop.getColourValue();
            *this = TilemapProperty{
                Color::From0_255(color.r, color.g, color.b, color.a)};
        } break;
        case tmx::Property::Type::File:
            *this = TilemapProperty{Path{prop.getFileValue()}};
            break;
        case tmx::Property::Type::Object:
        case tmx::Property::Type::Class:
            LOGW("[Tilemap]: not support object type property");
            break;
        case tmx::Property::Type::Undef:
            break;
    }
}

const int* TilemapProperty::AsInt() const {
    return std::get_if<int>(&m_data);
}

const bool* TilemapProperty::AsBool() const {
    return std::get_if<bool>(&m_data);
}

const std::string* TilemapProperty::AsString() const {
    return std::get_if<std::string>(&m_data);
}

const Path* TilemapProperty::AsPath() const {
    return std::get_if<Path>(&m_data);
}

const float* TilemapProperty::AsFloat() const {
    return std::get_if<float>(&m_data);
}

TilemapProperty::Type TilemapProperty::GetType() const {
    struct Visitor {
        Type operator()(int) const { return Type::Int; }

        Type operator()(float) const { return Type::Float; }

        Type operator()(const Color&) const { return Type::Color; }

        Type operator()(bool) const { return Type::Bool; }

        Type operator()(const Path&) const { return Type::File; }

        Type operator()(std::monostate) const { return Type::None; }
    };

    return std::visit(Visitor{}, m_data);
}

TilemapObject::TilemapObject(const std::string& name, const Circle& value,
                             const std::vector<TilemapProperty>& properties,
                             bool visiable) noexcept
    : m_data{value},
      m_name{name},
      m_properties{properties},
      m_visiable{visiable} {}

TilemapObject::TilemapObject(const std::string& name, const Rect& value,
                             const std::vector<TilemapProperty>& properties,
                             bool visiable) noexcept
    : m_data{value},
      m_name{name},
      m_properties{properties},
      m_visiable{visiable} {}

TilemapObject::TilemapObject(const std::string& name, const Polygon& value,
                             const std::vector<TilemapProperty>& properties,
                             bool visiable) noexcept
    : m_data{value},
      m_name{name},
      m_properties{properties},
      m_visiable{visiable} {}

TilemapObject::TilemapObject(const std::string& name, const Vec2& value,
                             const std::vector<TilemapProperty>& properties,
                             bool visiable) noexcept
    : m_data{value},
      m_name{name},
      m_properties{properties},
      m_visiable{visiable} {}

TilemapObject::TilemapObject(const tmx::Object& obj) {
    m_name = obj.getName();

    for (auto& prop : obj.getProperties()) {
        m_properties.emplace_back(TilemapProperty{prop});
    }

    m_visiable = obj.visible();
    switch (obj.getShape()) {
        case tmx::Object::Shape::Rectangle: {
            auto& aabb = obj.getAABB();
            m_data = Rect{
                Vec2{aabb.left + aabb.width * 0.5f,
                     aabb.top + aabb.height * 0.5f                    },
                Vec2{            aabb.width * 0.5f, aabb.height * 0.5f}
            };
        } break;
        case tmx::Object::Shape::Ellipse: {
            auto& aabb = obj.getAABB();
            TL_RETURN_IF_FALSE(aabb.width == aabb.height);
            m_data = Circle{
                Vec2{aabb.left + aabb.width * 0.5f,
                     aabb.top + aabb.height * 0.5f},
                aabb.width * 0.5f
            };
        } break;

        case tmx::Object::Shape::Point:
            m_data = Vec2{obj.getPosition().x, obj.getPosition().y};
            break;
        case tmx::Object::Shape::Polygon: {
            auto& points = obj.getPoints();
            auto& position = obj.getPosition();
            Polygon polygon;
            for (auto& p : points) {
                auto final_position = p + position;
                polygon.m_points.emplace_back(
                    Vec2{final_position.x, final_position.y});
            }
            auto flip = obj.getFlipFlags();
            auto& aabb = obj.getAABB();
            Vec2 center{aabb.left + aabb.width * 0.5f,
                        aabb.top + aabb.height * 0.5f};
            if (flip & FLIPPED_VERTICALLY_FLAG) {
                for (auto& p : polygon.m_points) {
                    p = Mirror(p, center, Vec2::X_UNIT);
                }
            }
            if (flip & FLIPPED_HORIZONTALLY_FLAG) {
                for (auto& p : polygon.m_points) {
                    p = Mirror(p, center, Vec2::Y_UNIT);
                }
            }
            m_data = polygon;
        } break;
        case tmx::Object::Shape::Polyline:
        case tmx::Object::Shape::Text:
            LOGW("[Tilemap]: not support polyline & text object");
            break;
    }
}

Circle* TilemapObject::AsCircle() {
    return const_cast<Circle*>(
        static_cast<const TilemapObject&>(*this).AsCircle());
}

Rect* TilemapObject::AsRect() {
    return const_cast<Rect*>(static_cast<const TilemapObject&>(*this).AsRect());
}

Polygon* TilemapObject::AsPolygon() {
    return const_cast<Polygon*>(
        static_cast<const TilemapObject&>(*this).AsPolygon());
}

Vec2* TilemapObject::AsPoint() {
    return const_cast<Vec2*>(
        static_cast<const TilemapObject&>(*this).AsPoint());
}

const Circle* TilemapObject::AsCircle() const {
    return std::get_if<Circle>(&m_data);
}

const Rect* TilemapObject::AsRect() const {
    return std::get_if<Rect>(&m_data);
}

const Polygon* TilemapObject::AsPolygon() const {
    return std::get_if<Polygon>(&m_data);
}

const Vec2* TilemapObject::AsPoint() const {
    return std::get_if<Vec2>(&m_data);
}

TilemapObject::Type TilemapObject::GetType() const {
    struct Visitor {
        Type operator()(const Circle&) const { return Type::Circle; }

        Type operator()(const Rect&) const { return Type::Rect; }

        Type operator()(const Polygon&) const { return Type::Polygon; }

        Type operator()(const Vec2&) const { return Type::Point; }

        Type operator()(std::monostate) const { return Type::None; }
    };

    return std::visit(Visitor{}, m_data);
}

bool TilemapObject::IsVisiable() const {
    return m_visiable;
}

const std::string& TilemapObject::GetName() const {
    return m_name;
}

TilemapObjectLayer::TilemapObjectLayer(const std::string& name,
                                       const tmx::ObjectGroup& layer)
    : TilemapLayer(name, Type::Object) {
    parse(layer);
}

const std::vector<TilemapObject>& TilemapObjectLayer::GetObjects() const {
    return m_objects;
}

void TilemapObjectLayer::parse(const tmx::ObjectGroup& layer) {
    for (auto& prop : layer.getProperties()) {
        m_properties.emplace_back(TilemapProperty{prop});
    }

    for (auto& object : layer.getObjects()) {
        m_objects.emplace_back(TilemapObject{object});
    }
}

TilemapImageLayer::TilemapImageLayer(const std::string& name,
                                     const tmx::ImageLayer& layer,
                                     const Path& dir)
    : TilemapLayer{name, Type::Image} {
    parse(layer, dir);
}

ImageHandle TilemapImageLayer::GetImage() const {
    return m_image;
}

const Vec2& TilemapImageLayer::GetPosition() const {
    return m_position;
}

void TilemapImageLayer::parse(const tmx::ImageLayer& layer, const Path& dir) {
    for (auto& prop : layer.getProperties()) {
        m_properties.emplace_back(TilemapProperty{prop});
    }

    auto image_path = layer.getImagePath();

    m_image =
        COMMON_CONTEXT.m_assets_manager->GetManager<ImageBase>().Load(image_path);
    m_position.x = layer.getOffset().x;
    m_position.y = layer.getOffset().y;
}

Tileset::Tileset(const tmx::Tileset& tileset) {
    parse(tileset);
}

void Tileset::parse(const tmx::Tileset& tileset) {
    auto path = tileset.getImagePath();
    m_image = COMMON_CONTEXT.m_assets_manager->GetManager<ImageBase>().Load(path);
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

const Vec2& Tileset::GetTileSize() const {
    return m_tile_size;
}

Tilemap::Tilemap(const Path& filename) : m_filename{filename} {
    parse(filename);
}

Tilemap::Tilemap(const Tilemap& o)
    : m_tile_size{o.m_tile_size},
      m_tilesets{o.m_tilesets},
      m_filename{o.m_filename} {
    for (auto& layer : o.m_layers) {
        switch (layer->GetType()) {
            case TilemapLayer::Type::Tiled:
                m_layers.emplace_back(
                    std::make_unique<TilemapTileLayer>(*layer->AsTiledLayer()));
                break;
            case TilemapLayer::Type::Object:
                m_layers.emplace_back(std::make_unique<TilemapObjectLayer>(
                    *layer->AsObjectLayer()));
            case TilemapLayer::Type::Image:
                m_layers.emplace_back(std::make_unique<TilemapImageLayer>(
                    *layer->AsImageLayer()));
                break;
        }
    }
}

Tilemap& Tilemap::operator=(const Tilemap& o) {
    TL_RETURN_VALUE_IF_FALSE(&o != this, *this);

    m_tile_size = o.m_tile_size;
    m_tilesets = o.m_tilesets;
    m_filename = o.m_filename;
    for (auto& layer : m_layers) {
        switch (layer->GetType()) {
            case TilemapLayer::Type::Tiled:
                m_layers.emplace_back(
                    std::make_unique<TilemapTileLayer>(*layer->AsTiledLayer()));
                break;
            case TilemapLayer::Type::Object:
                m_layers.emplace_back(std::make_unique<TilemapObjectLayer>(
                    *layer->AsObjectLayer()));
            case TilemapLayer::Type::Image:
                m_layers.emplace_back(std::make_unique<TilemapImageLayer>(
                    *layer->AsImageLayer()));
                break;
        }
    }

    return *this;
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
            m_layers.emplace_back(std::make_unique<TilemapTileLayer>(
                tile_layer.getName(), tile_layer));
        }
        if (layer->getType() == tmx::Layer::Type::Object) {
            const auto& object_layer = layer->getLayerAs<tmx::ObjectGroup>();
            m_layers.emplace_back(std::make_unique<TilemapObjectLayer>(
                object_layer.getName(), object_layer));
        }
        if (layer->getType() == tmx::Layer::Type::Image) {
            const auto& image_layer = layer->getLayerAs<tmx::ImageLayer>();
            m_layers.emplace_back(std::make_unique<TilemapImageLayer>(
                image_layer.getName(), image_layer, filename.parent_path()));
        }
    }

    const auto& tilesets = map.getTilesets();
    for (const auto& tileset : tilesets) {
        m_tilesets.emplace_back(tileset);
    }
}

TilemapHandle TilemapManager::Load(const Path& filename, bool force) {
    if (auto handle = Find(filename); handle && !force) {
        return handle;
    }
    return store(&filename, UUID::CreateV4(),
                 std::make_unique<Tilemap>(filename));
}
