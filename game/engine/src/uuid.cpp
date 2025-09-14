#include "uuid.h"
#include "engine/uuid.hpp"

#include "engine/log.hpp"

#include <chrono>

UUIDv4 UUIDv4::Create() {
    static std::mt19937 generator(
        std::chrono::system_clock::now().time_since_epoch().count());
    auto uuid = uuids::uuid_random_generator{generator}();
    auto bytes = uuid.as_bytes();
    UUIDv4 result;
    std::copy(bytes.begin(), bytes.end(), result.m_data.begin());
    return result;
}

UUIDv4 UUIDv4::CreateFromString(const std::string& str) {
    UUIDv4 result;
    if (auto uuid = uuids::uuid::from_string(str); uuid) {
        auto bytes = uuid->as_bytes();
        std::copy(bytes.begin(), bytes.end(), result.m_data.begin());
    } else {
        LOGE("invalid UUID string: {}", str);
    }
    return result;
}

UUIDv4::operator bool() const {
    return !(m_data[0] == std::byte{0} && m_data[1] == std::byte{0} &&
             m_data[2] == std::byte{0} && m_data[3] == std::byte{0});
}

bool UUIDv4::operator==(UUIDv4 const& o) const {
    return m_data[0] == o.m_data[0] && m_data[1] == o.m_data[1] &&
           m_data[2] == o.m_data[2] && m_data[3] == o.m_data[3];
}

bool UUIDv4::operator!=(UUIDv4 const& o) const {
    return !(*this == o);
}

std::string UUIDv4::ToString() const {
    uuids::uuid uuid{
        {static_cast<uint8_t>(m_data[0]), static_cast<uint8_t>(m_data[1]),
         static_cast<uint8_t>(m_data[2]), static_cast<uint8_t>(m_data[3])}
    };
    return uuids::to_string<char>(uuid);
}

std::ostream& operator<<(std::ostream& o, const UUIDv4& uuid) {
    o << uuid.ToString();
    return o;
}

UUIDv4::UUIDv4() {
    m_data.fill(std::byte{0});
}

std::hash<UUIDv4>::result_type std::hash<UUIDv4>::operator()(
    argument_type const& uuid) const noexcept {
    uuids::uuid id{
        {static_cast<uint8_t>(uuid.m_data[0]),
         static_cast<uint8_t>(uuid.m_data[1]),
         static_cast<uint8_t>(uuid.m_data[2]),
         static_cast<uint8_t>(uuid.m_data[3])}
    };
    return std::hash<uuids::uuid>{}(id);
}