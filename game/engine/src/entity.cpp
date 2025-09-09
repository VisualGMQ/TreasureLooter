#include "engine/entity.hpp"

#include <type_traits>

constexpr bool NullEntity::operator==(Entity entity) const {
    return static_cast<std::underlying_type_t<Entity>>(entity) == 0;
}

constexpr bool NullEntity::operator!=(Entity entity) const {
    return !(*this == entity);
}

constexpr bool NullEntity::operator==(NullEntity) const {
    return true;
}

constexpr bool NullEntity::operator!=(NullEntity) const {
    return false;
}

std::ostream& operator<<(std::ostream& os, const Entity& e) {
    os << "Entity(" << static_cast<std::underlying_type_t<Entity>>(e) << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const NullEntity& e) {
    os << "NullEntity";
    return os;
}