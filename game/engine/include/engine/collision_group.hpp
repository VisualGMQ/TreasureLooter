#pragma once
#include "schema/collision_group_schema.hpp"
#include <type_traits>

class CollisionGroup {
public:
    using underlying_type = std::underlying_type_t<CollisionGroupType>;

    CollisionGroup() = default;
    CollisionGroup(std::initializer_list<CollisionGroupType>&&);

    void Add(CollisionGroupType);
    void Remove(CollisionGroupType);
    [[nodiscard]] bool Has(CollisionGroupType) const;
    void Clear();

    [[nodiscard]] bool CanCollision(CollisionGroup) const;

    [[nodiscard]] underlying_type GetUnderlying() const;
    void SetUnderlying(underlying_type);

private:
    underlying_type m_collision_group{};
};
