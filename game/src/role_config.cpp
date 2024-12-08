#include "role_config.hpp"
#include "context.hpp"

namespace tl {

RoleConfig RoleConfig::Null;

RoleConfig::operator bool() const noexcept {
    return enable;
}

RoleConfig::Type GetRoleTypeByName(const std::string& name) {
    static std::unordered_map<std::string, RoleConfig::Type> map = {
        {"ordinary", RoleConfig::Type::Ordinary},
        {   "ninja",    RoleConfig::Type::Ninja},
    };

    auto it = map.find(name);
    if (it == map.end()) {
        return RoleConfig::Type::Unknown;
    }
    return it->second;
}

const RoleConfig* RoleConfigManager::Load(const std::string& filename,
                                           const std::string& name) {
    tinyxml2::XMLDocument doc;
    void* fileContent = SDL_LoadFile(filename.c_str(), nullptr);
    tinyxml2::XMLError err = doc.Parse((const char*)fileContent);
    TL_RETURN_NULL_IF_FALSE_LOGE(!err, "%s role parse error: %s",
                                 filename.c_str(), GetXMLErrStr(err));

    auto elem = doc.FirstChildElement("role");
    TL_RETURN_NULL_IF_FALSE_LOGE(elem, "%s role parse failed",
                                 filename.c_str());

    RoleConfig role;
    TL_RETURN_NULL_IF_FALSE(parseRole(*elem, role));

    auto result = datas_.emplace(name, std::move(role));
    TL_RETURN_NULL_IF_FALSE_LOGE(result.second, "%s emplace failed",
                                 filename.c_str());

    return &result.first->second;
}

const RoleConfig& RoleConfigManager::Find(const std::string& name) const {
    auto it = datas_.find(name);
    if (it == datas_.end()) {
        return RoleConfig::Null;
    }
    return it->second;
}

bool RoleConfigManager::parseRole(const tinyxml2::XMLElement& elem,
                                  RoleConfig& role) const {
    if (auto templateElem = elem.FirstChildElement("template")) {
        tinyxml2::XMLDocument doc;
        std::string filename =
            std::string{"assets/game/role/template/"} + templateElem->GetText() + ".xml";
        void* fileContent = SDL_LoadFile(filename.c_str(), nullptr);
        tinyxml2::XMLError err = doc.Parse((const char*)fileContent);
        if (err) {
            LOGE("%s parse failed: %s", templateElem->GetText(),
                 GetXMLErrStr(err));
            return role;
        }

        auto roleElem = doc.FirstChildElement("role");
        if (!roleElem) {
            LOGE("%s load failed: invalid content", filename.c_str());
            return role;
        }
        if (!parseRole(*roleElem, role)) {
            return false;
        }
    }

    if (auto typeElem = elem.FirstChildElement("type")) {
        role.type = GetRoleTypeByName(typeElem->GetText());
        if (role.type == RoleConfig::Type::Unknown) {
            LOGE("unknown role type");
            role.type = RoleConfig::Type::Ordinary;
        }
    }

    if (auto textureElem = elem.FirstChildElement("texture")) {
        role.roleTexture =
            Context::GetInst().textureMgr->Find(textureElem->GetText());
    }

    if (auto regionElem = elem.FirstChildElement("stand_down_region")) {
        Rect region;
        ParseFloat(regionElem->GetText(), (float*)(&region), 4);
        role.standDownRegion = region;
    }

    if (auto regionElem = elem.FirstChildElement("stand_up_region")) {
        Rect region;
        ParseFloat(regionElem->GetText(), (float*)(&region), 4);
        role.standUpRegion = region;
    }

    if (auto regionElem = elem.FirstChildElement("stand_left_region")) {
        Rect region;
        ParseFloat(regionElem->GetText(), (float*)(&region), 4);
        role.standLeftRegion = region;
    }

    if (auto animElem = elem.FirstChildElement("walk_left_anim")) {
        role.walkLeftAnim =
            Context::GetInst().animMgr->Find(animElem->GetText());
    }

    if (auto animElem = elem.FirstChildElement("walk_down_anim")) {
        role.walkDownAnim =
            Context::GetInst().animMgr->Find(animElem->GetText());
    }

    if (auto animElem = elem.FirstChildElement("walk_up_anim")) {
        role.walkUpAnim = Context::GetInst().animMgr->Find(animElem->GetText());
    }

    if (auto speedElem = elem.FirstChildElement("speed")) {
        ParseFloat(speedElem->GetText(), &role.speed, 1);
    }

    role.enable = true;

    return true;
}

}

