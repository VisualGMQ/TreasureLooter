#pragma once
#include "pch.hpp"
#include <type_traits>

namespace tl {

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
class Flags {
public:
    using underlying_type = std::underlying_type_t<T>;

    Flags() = default;
    Flags(T value) : value_{static_cast<underlying_type>(value)} {}

    Flags(std::underlying_type_t<T> value) : value_{value} {}

    Flags(const Flags&) = default;
    Flags(Flags&&) = default;

    Flags operator|(T o) const { return Flags{value_ | static_cast<underlying_type>(o)}; }

    Flags operator&(T o) const { return Flags{value_ & static_cast<underlying_type>(o)}; }

    Flags& operator|=(T o) {
        value_ |= static_cast<underlying_type>(o);
        return *this;
    }

    Flags& operator&=(T o) {
        value_ &= static_cast<underlying_type>(o);
        return *this;
    }

    operator T() const { return static_cast<T>(value_); }

private:
    underlying_type value_{};
};

}  // namespace tl
