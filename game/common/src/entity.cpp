#include "common/entity.hpp"

#include <type_traits>

constexpr bool NullEntity::operator==(LogicEntity entity) const {
    return static_cast<std::underlying_type_t<LogicEntity>>(entity) == 0;
}

constexpr bool NullEntity::operator!=(LogicEntity entity) const {
    return !(*this == entity);
}

constexpr bool NullEntity::operator==(PresentEntity entity) const {
    return static_cast<std::underlying_type_t<PresentEntity>>(entity) == 0;
}

constexpr bool NullEntity::operator!=(PresentEntity entity) const {
    return !(*this == entity);
}

constexpr bool NullEntity::operator==(NullEntity) const {
    return true;
}

constexpr bool NullEntity::operator!=(NullEntity) const {
    return false;
}

std::ostream& operator<<(std::ostream& os, const LogicEntity& e) {
    os << "LogicEntity(" << static_cast<std::underlying_type_t<LogicEntity>>(e)
       << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const PresentEntity& e) {
    os << "PresentEntity("
       << static_cast<std::underlying_type_t<PresentEntity>>(e) << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const NullEntity& e) {
    os << "NullEntity";
    return os;
}
