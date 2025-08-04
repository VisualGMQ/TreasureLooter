#pragma once
#include "spdlog/fmt/bundled/os.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include <cstdint>
#include <iostream>

enum class Entity : uint32_t {};

struct NullEntity {
    constexpr bool operator==(Entity) const;
    constexpr bool operator!=(Entity) const;
    constexpr bool operator==(NullEntity) const;
    constexpr bool operator!=(NullEntity) const;
};

constexpr NullEntity null_entity;

std::ostream& operator<<(std::ostream& os, const Entity& e);
std::ostream& operator<<(std::ostream& os, const NullEntity& e);

// for spdlog output
template <>
struct fmt::formatter<Entity> : fmt::ostream_formatter {};
