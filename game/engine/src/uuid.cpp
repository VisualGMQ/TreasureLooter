#include "engine/uuid.hpp"
#include "uuid.h"

#include "engine/log.hpp"

#include <chrono>

UUID UUID::CreateV4() {
    static std::mt19937 generator(
        std::chrono::system_clock::now().time_since_epoch().count());
    auto uuid = uuids::uuid_random_generator{generator}();
    auto bytes = uuid.as_bytes();
    UUID result;
    std::copy(bytes.begin(), bytes.end(), result.m_data.begin());
    return result;
}

UUID UUID::CreateFromString(const std::string& str) {
    UUID result;
    if (auto uuid = uuids::uuid::from_string(str); uuid) {
        auto bytes = uuid->as_bytes();
        std::copy(bytes.begin(), bytes.end(), result.m_data.begin());
    } else {
        LOGE("invalid UUID string: {}", str);
    }
    return result;
}

UUID::operator bool() const {
    return !(m_data[0] == std::byte{0} && m_data[1] == std::byte{0} &&
             m_data[2] == std::byte{0} && m_data[3] == std::byte{0} &&
             m_data[4] == std::byte{0} && m_data[5] == std::byte{0} &&
             m_data[6] == std::byte{0} && m_data[7] == std::byte{0} &&
             m_data[8] == std::byte{0} && m_data[9] == std::byte{0} &&
             m_data[10] == std::byte{0} && m_data[11] == std::byte{0} &&
             m_data[12] == std::byte{0} && m_data[13] == std::byte{0} &&
             m_data[14] == std::byte{0} && m_data[15] == std::byte{0});
}

bool UUID::operator==(UUID const& o) const {
    return m_data[0] == o.m_data[0] &&
           m_data[1] == o.m_data[1] &&
           m_data[2] == o.m_data[2] &&
           m_data[3] == o.m_data[3] &&
           m_data[4] == o.m_data[4] &&
           m_data[5] == o.m_data[5] &&
           m_data[6] == o.m_data[6] &&
           m_data[7] == o.m_data[7] &&
           m_data[8] == o.m_data[8] &&
           m_data[9] == o.m_data[9] &&
           m_data[10] == o.m_data[10] &&
           m_data[11] == o.m_data[11] &&
           m_data[12] == o.m_data[12] &&
           m_data[13] == o.m_data[13] &&
           m_data[14] == o.m_data[14] &&
           m_data[15] == o.m_data[15];
}

bool UUID::operator!=(UUID const& o) const {
    return !(*this == o);
}

std::string UUID::ToString() const {
    uuids::uuid uuid{
        {
            static_cast<uint8_t>(m_data[0]),
            static_cast<uint8_t>(m_data[1]),
            static_cast<uint8_t>(m_data[2]),
            static_cast<uint8_t>(m_data[3]),
            static_cast<uint8_t>(m_data[4]),
            static_cast<uint8_t>(m_data[5]),
            static_cast<uint8_t>(m_data[6]),
            static_cast<uint8_t>(m_data[7]),
            static_cast<uint8_t>(m_data[8]),
            static_cast<uint8_t>(m_data[9]),
            static_cast<uint8_t>(m_data[10]),
            static_cast<uint8_t>(m_data[11]),
            static_cast<uint8_t>(m_data[12]),
            static_cast<uint8_t>(m_data[13]),
            static_cast<uint8_t>(m_data[14]),
            static_cast<uint8_t>(m_data[15]),
        }
    };
    return uuids::to_string<char>(uuid);
}

std::ostream& operator<<(std::ostream& o, const UUID& uuid) {
    o << uuid.ToString();
    return o;
}

UUID::UUID() {
    m_data.fill(std::byte{0});
}

std::hash<UUID>::result_type std::hash<UUID>::operator()(
    argument_type const& uuid) const noexcept {
    uuids::uuid id{
        {
         static_cast<uint8_t>(uuid.m_data[0]),
         static_cast<uint8_t>(uuid.m_data[1]),
         static_cast<uint8_t>(uuid.m_data[2]),
         static_cast<uint8_t>(uuid.m_data[3]),
         static_cast<uint8_t>(uuid.m_data[4]),
         static_cast<uint8_t>(uuid.m_data[5]),
         static_cast<uint8_t>(uuid.m_data[6]),
         static_cast<uint8_t>(uuid.m_data[7]),
         static_cast<uint8_t>(uuid.m_data[8]),
         static_cast<uint8_t>(uuid.m_data[9]),
         static_cast<uint8_t>(uuid.m_data[10]),
         static_cast<uint8_t>(uuid.m_data[11]),
         static_cast<uint8_t>(uuid.m_data[12]),
         static_cast<uint8_t>(uuid.m_data[13]),
         static_cast<uint8_t>(uuid.m_data[14]),
         static_cast<uint8_t>(uuid.m_data[15]),
         }
    };
    return std::hash<uuids::uuid>{}(id);
}