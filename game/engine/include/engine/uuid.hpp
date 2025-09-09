#pragma once

#include <array>
#include <cstddef>
#include <string>

struct UUID {
    friend class std::hash<UUID>;

    static UUID CreateV4();
    static UUID CreateFromString(const std::string &);

    UUID();

    operator bool() const;
    bool operator==(UUID const &) const;
    bool operator!=(UUID const &) const;

    std::string ToString() const;

private:
    std::array<std::byte, 16> m_data;
};

std::ostream &operator<<(std::ostream &o, const UUID &uuid);

namespace std {
template <>
struct hash<UUID> {
    using argument_type = UUID;
    using result_type = std::size_t;

    [[nodiscard]] result_type operator()(
        argument_type const &uuid) const noexcept;
};
}  // namespace std