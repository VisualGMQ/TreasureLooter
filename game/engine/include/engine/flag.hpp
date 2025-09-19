#pragma once
#include <type_traits>

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
class Flags {
public:
    using enum_type = T;
    using underlying_type = std::underlying_type_t<T>;

    Flags() = default;

    Flags(T value) : m_value{static_cast<underlying_type>(value)} {}

    Flags(std::underlying_type_t<T> value) : m_value{value} {}

    Flags(const Flags&) = default;
    Flags(Flags&&) = default;

    Flags operator|(T o) const {
        return m_value | static_cast<underlying_type>(o);
    }

    Flags operator&(T o) const {
        return m_value & static_cast<underlying_type>(o);
    }

    Flags& operator=(const Flags&) = default;

    Flags& operator=(T value) {
        m_value = static_cast<underlying_type>(value);
        return *this;
    }

    Flags& operator|=(T o) {
        m_value |= static_cast<underlying_type>(o);
        return *this;
    }

    Flags& operator&=(T o) {
        m_value &= static_cast<underlying_type>(o);
        return *this;
    }

    Flags operator~() const noexcept { return ~m_value; }

    void Remove(T o) {
        m_value &= ~static_cast<underlying_type>(o);
    }

    operator T() const { return static_cast<T>(m_value); }

    explicit operator bool() const { return static_cast<underlying_type>(m_value) != 0; }

    underlying_type Value() const { return m_value; }

private:
    underlying_type m_value{};
};

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
typename Flags<T>::underlying_type operator&(T o, Flags<T>& flags) {
    return flags & o;
}

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
typename Flags<T>::underlying_type operator|(T o, Flags<T>& flags) {
    return flags | o;
}
