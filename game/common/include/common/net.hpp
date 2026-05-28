#pragma once
#include <string>

typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;
typedef struct _ENetAddress ENetAddress;

template <typename T>
class NetMsg {
public:
    using message_type = T;

    NetMsg() = default;

    explicit NetMsg(ENetPeer* peer, const T& data)
        : m_data{data}, m_peer{peer} {}

    explicit NetMsg(ENetPeer* peer, T&& data)
        : m_data{std::move(data)}, m_peer{peer} {}

    T* operator->() { return &m_data; }

    const T* operator->() const { return &m_data; }

    ENetPeer* m_peer{};

    T& Payload() { return m_data; }
    const T& Payload() const { return m_data; }

private:
    T m_data;
};

struct NetMsgError {
    std::string m_error_msg;
};
