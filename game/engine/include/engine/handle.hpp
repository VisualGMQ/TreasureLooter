#pragma once
#include "uuid.hpp"
#include "asset.hpp"
#include "path.hpp"

template <typename T>
class Handle {
public:
    using underlying_type = T;

    Handle() = default;

    Handle(nullptr_t) {}

    Handle(UUIDv4 uuid, T* data, IAssetManager* manager)
        : m_data{data}, m_uuid{uuid}, m_manager{manager} {}

    operator bool() const { return m_data; }

    T* operator->() { return m_data; }

    const T* operator->() const { return m_data; }

    friend T& operator*(Handle& handle) { return *handle.m_data; }

    friend const T& operator*(const Handle& handle) { return *handle.m_data; }

    bool operator==(const Handle& o) noexcept { return m_data == o.m_data; }

    bool operator!=(const Handle& o) noexcept { return !(m_data == o.m_data); }

    void Reset() { m_data = nullptr; }

    const UUIDv4& GetUUID() const { return m_uuid; }

    const Path* GetFilename() const {
        return m_manager ? m_manager->GetFilename(m_uuid) : nullptr;
    }

    T* Get() { return m_data; }

    const T* Get() const { return m_data; }

private:
    T* m_data{};
    IAssetManager* m_manager{};
    UUIDv4 m_uuid;
};

namespace internal {

template <typename T>
struct is_handle {
    static constexpr bool value = false;
};

template <typename T>
struct is_handle<Handle<T>> {
    static constexpr bool value = true;
};
}

template <typename T>
static constexpr bool is_handle_v = internal::is_handle<T>::value;

namespace std {
template <typename T>
struct hash<Handle<T>> {
    using argument_type = Handle<T>;
    using result_type = std::size_t;

    [[nodiscard]] result_type operator()(
        argument_type const& handle) const noexcept {
        return hash<UUIDv4>()(handle.GetUUID());
    }
};
}  // namespace std
