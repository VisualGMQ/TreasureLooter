#pragma once
#include "common/flag.hpp"
#include "common/image.hpp"
#include "common/manager.hpp"
#include "common/path.hpp"
#include "common/physics.hpp"
#include "schema/common.hpp"
#include "schema/flip.hpp"
#include "tmxlite/ImageLayer.hpp"
#include "tmxlite/Layer.hpp"
#include "tmxlite/Map.hpp"
#include "tmxlite/ObjectGroup.hpp"
#include "tmxlite/Property.hpp"
#include <variant>

class TilemapTileLayer;
class TilemapLayerDefinition;
class DrawOrder;

struct TilemapProperty {
    enum class Type {
        None,
        Bool,
        Color,
        Float,
        File,
        Int,
        String,
        // Object // not support yet
    };

    explicit TilemapProperty(int);
    explicit TilemapProperty(float);
    explicit TilemapProperty(bool);
    explicit TilemapProperty(const Color&);
    explicit TilemapProperty(const Path&);
    explicit TilemapProperty(const std::string&);
    explicit TilemapProperty(const tmx::Property&);

    const int* AsInt() const;
    const bool* AsBool() const;
    const std::string* AsString() const;
    const Path* AsPath() const;
    const float* AsFloat() const;

    Type GetType() const;

private:
    using Data = std::variant<std::monostate, int, bool, float, Color, Path,
                              std::string>;

    Data m_data;
};

class TilemapLayer {
public:
    enum class Type {
        Tiled,
        Object,
        Image,
        // Group, not support currently
    };

    TilemapLayer(const std::string& name, Type);

    virtual ~TilemapLayer() = default;

    const TilemapTileLayer* AsTiledLayer() const;
    const class TilemapImageLayer* AsImageLayer() const;
    const class TilemapObjectLayer* AsObjectLayer() const;

    Type GetType() const;

    const std::vector<TilemapProperty>& GetProperties() const;
    std::string_view GetName() const;

protected:
    std::vector<TilemapProperty> m_properties;

private:
    Type m_type;
    std::string m_name;
};

class TilemapTileLayer : public TilemapLayer {
public:
    struct Tile {
        uint32_t m_gid;
        Flags<Flip> m_flip = Flip::None;
    };

    explicit TilemapTileLayer(const std::string& name, const tmx::TileLayer&);

    const Tile& GetTile(int x, int y) const;
    const Vec2& GetSize() const;

private:
    void parse(const tmx::TileLayer&);

    std::vector<Tile> m_tiles;
    Vec2 m_size;
};

class TilemapObject {
public:
    enum class Type {
        None,
        Point,
        Circle,
        Rect,
        Polygon,
        // Text, not support currently
    };

    TilemapObject() = default;
    TilemapObject(const std::string& name, const Circle&,
                  const std::vector<TilemapProperty>&, bool visiable) noexcept;
    TilemapObject(const std::string& name, const Rect&,
                  const std::vector<TilemapProperty>&, bool visiable) noexcept;
    TilemapObject(const std::string& name, const Polygon&,
                  const std::vector<TilemapProperty>&, bool visiable) noexcept;
    TilemapObject(const std::string& name, const Vec2&,
                  const std::vector<TilemapProperty>&, bool visiable) noexcept;
    explicit TilemapObject(const tmx::Object&);

    class Circle* AsCircle();
    class Rect* AsRect();
    class Polygon* AsPolygon();
    Vec2* AsPoint();

    const class Circle* AsCircle() const;
    const class Rect* AsRect() const;
    const class Polygon* AsPolygon() const;
    const Vec2* AsPoint() const;

    Type GetType() const;
    bool IsVisiable() const;

    const std::string& GetName() const;

private:
    using Data = std::variant<std::monostate, Circle, Rect, Polygon, Vec2>;

    bool m_visiable{false};
    Data m_data;
    std::string m_name;
    std::vector<TilemapProperty> m_properties;
};

class TilemapObjectLayer : public TilemapLayer {
public:
    explicit TilemapObjectLayer(const std::string& name,
                                const tmx::ObjectGroup&);

    const std::vector<TilemapObject>& GetObjects() const;

private:
    std::vector<TilemapObject> m_objects;

    void parse(const tmx::ObjectGroup&);
};

class TilemapImageLayer : public TilemapLayer {
public:
    TilemapImageLayer(const std::string& name, const tmx::ImageLayer&,
                      const Path& dir);

    ImageHandle GetImage() const;
    const Vec2& GetPosition() const;

private:
    ImageHandle m_image;
    Vec2 m_position;

    void parse(const tmx::ImageLayer&, const Path& dir);
};

class Tileset;

struct Tile {
    ImageHandle m_image;
    Region m_region;
    uint32_t m_id;
    Rect m_collision_rect;
    Vec2 m_tile_size;
};

class Tileset {
public:
    Tileset(const tmx::Tileset&);

    const Tile& GetTile(uint32_t gid) const;

    bool HasTile(uint32_t gid) const;
    const Vec2& GetTileSize() const;

private:
    void parse(const tmx::Tileset& tileset);

    ImageHandle m_image;
    uint32_t m_margin;
    uint32_t m_spacing;
    std::vector<std::uint32_t> m_tile_index;
    std::vector<Tile> m_tiles;
    uint32_t m_firstgid;
    uint32_t m_lastgid;
    Vec2 m_tile_size;
};

class Tilemap {
public:
    explicit Tilemap(const Path& filename);

    Tilemap(const Tilemap& o);
    Tilemap(Tilemap&&) = default;
    Tilemap& operator=(const Tilemap&);
    Tilemap& operator=(Tilemap&&) = default;

    auto& GetLayers() const { return m_layers; }

    auto& GetTileset() const { return m_tilesets; }

    const Tile* GetTile(uint32_t gid) const;

    const Vec2& GetTileSize() const;

    const Path& GetFilename() const;

private:
    void parse(const Path& filename);

    std::vector<std::unique_ptr<TilemapLayer>> m_layers;
    std::vector<Tileset> m_tilesets;
    Vec2 m_tile_size;
    Path m_filename;
};

template <>
class AssetSLInfo<Tilemap> {
public:
    static constexpr bool CanEmbed = false;
};

using TilemapHandle = Handle<Tilemap>;

class TilemapManager : public AssetManagerBase<Tilemap> {
public:
    TilemapHandle Load(const Path& filename, bool force = false) override;
};
