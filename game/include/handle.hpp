#pragma once
#include "uuid.hpp"

template <typename T>
class Handle {
public:
    Handle() = default;

    Handle(nullptr_t) {}

    Handle(UUID uuid, T* data) : m_data{data}, m_uuid{uuid} {}

    operator bool() const { return m_data; }

    T* operator->() { return m_data; }

    const T* operator->() const { return m_data; }

    friend T& operator*(Handle& handle) { return *handle.m_data; }

    friend const T& operator*(const Handle& handle) { return *handle.m_data; }

    bool operator==(const Handle& o) noexcept { return m_data == o.m_data; }

    bool operator!=(const Handle& o) noexcept { return !(m_data == o.m_data); }

    void Reset() { m_data = nullptr; }

    const UUID& GetUUID() const { return m_uuid; }

    T* Get() { return m_data; }

    const T* Get() const { return m_data; }

private:
    T* m_data{};

    UUID m_uuid;
};