#pragma once
#include "spdlog/fmt/bundled/os.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include <cstdint>
#include <iostream>

/**
 * logic entity: the authority of the game logic, exists on both client and
 * server.
 */
enum class LogicEntity : uint32_t {};

/**
 * present entity: render-only entity, only exists on client. its value is
 * always the corresponding logic entity plus PresentEntityOffset.
 */
enum class PresentEntity : uint32_t {};

constexpr std::underlying_type_t<PresentEntity> PresentEntityOffset = 1000000;

struct NullEntity {
    constexpr bool operator==(LogicEntity) const;
    constexpr bool operator!=(LogicEntity) const;
    constexpr bool operator==(PresentEntity) const;
    constexpr bool operator!=(PresentEntity) const;
    constexpr bool operator==(NullEntity) const;
    constexpr bool operator!=(NullEntity) const;

    operator LogicEntity() const {
        return static_cast<LogicEntity>(0);
    }

    operator PresentEntity() const {
        return static_cast<PresentEntity>(0);
    }
};

constexpr NullEntity null_entity;

constexpr PresentEntity ToPresentEntity(LogicEntity entity) {
    return static_cast<PresentEntity>(
        static_cast<std::underlying_type_t<LogicEntity>>(entity) +
        PresentEntityOffset);
}

constexpr LogicEntity ToLogicEntity(PresentEntity entity) {
    return static_cast<LogicEntity>(
        static_cast<std::underlying_type_t<PresentEntity>>(entity) -
        PresentEntityOffset);
}

std::ostream& operator<<(std::ostream& os, const LogicEntity& e);
std::ostream& operator<<(std::ostream& os, const PresentEntity& e);
std::ostream& operator<<(std::ostream& os, const NullEntity& e);

// for spdlog output
template <>
struct fmt::formatter<LogicEntity> : fmt::ostream_formatter {};

template <>
struct fmt::formatter<PresentEntity> : fmt::ostream_formatter {};
