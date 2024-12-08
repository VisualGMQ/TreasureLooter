#pragma once
#include "animation.hpp"
#include "texture.hpp"

namespace tl {

struct RoleConfig {
    static RoleConfig Null;
    
    enum class Type {
        Unknown = 0,
        Ordinary,
        Ninja,
    } type = Type::Ordinary;

    Texture* roleTexture = nullptr;
    Rect standDownRegion;
    Rect standUpRegion;
    Rect standLeftRegion;
    Animation* walkLeftAnim = nullptr;
    Animation* walkDownAnim = nullptr;
    Animation* walkUpAnim = nullptr;
    float speed = 0.4;
    bool enable = false;

    operator bool() const noexcept;
};

RoleConfig::Type GetRoleTypeByName(const std::string& name);

class RoleConfigManager {
public:
    const RoleConfig* Load(const std::string& filename,
                           const std::string& name);
    const RoleConfig& Find(const std::string& name) const;

private:
    std::unordered_map<std::string, RoleConfig> datas_;
    bool parseRole(const tinyxml2::XMLElement&, RoleConfig& role) const;
};

struct Weapon {
    Animation* attackAnim = nullptr;
};

}  // namespace tl
