#pragma once

#include <array>
#include <cstddef>
#include <string>

struct UUIDv4 {
    friend class std::hash<UUIDv4>;

    static UUIDv4 Create();
    static UUIDv4 CreateFromString(const std::string &);

    UUIDv4();

    operator bool() const;
    bool operator==(UUIDv4 const &) const;
    bool operator!=(UUIDv4 const &) const;

    std::string ToString() const;

private:
    std::array<std::byte, 16> m_data;
};

std::ostream &operator<<(std::ostream &o, const UUIDv4 &uuid);

namespace std {
template <>
struct hash<UUIDv4> {
    using argument_type = UUIDv4;
    using result_type = std::size_t;

    [[nodiscard]] result_type operator()(
        argument_type const &uuid) const noexcept;
};
}  // namespace std