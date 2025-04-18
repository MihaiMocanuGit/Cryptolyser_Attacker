#include "ServerConnection.hpp"

#include "Cryptolyser_Common/connection_data_types.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>

template <bool KnownKey>
constexpr uint32_t ServerConnection<KnownKey>::DATA_MAX_SIZE {CONNECTION_DATA_MAX_SIZE};

template <bool KnownKey>
void ServerConnection<KnownKey>::m_swap(ServerConnection &server1, ServerConnection &server2)
{
    std::swap(server1.m_ip, server2.m_ip);
    std::swap(server1.m_port, server2.m_port);
    std::swap(server1.m_sock, server2.m_sock);
    std::swap(server1.m_isConnectionActive, server2.m_isConnectionActive);
}

template <bool KnownKey>
void ServerConnection<KnownKey>::m_closeSocket()
{
    close(m_sock);
    m_isConnectionActive = false;
}

template <bool KnownKey>
ServerConnection<KnownKey>::ServerConnection(std::string_view ip, uint16_t port)
    : m_ip {ip}, m_port {port}
{
}

template <bool KnownKey>
bool ServerConnection<KnownKey>::connect()
{
    m_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sock < 0)
    {
        std::cerr << "Error creating socket" << std::endl;
        return false;
    }

    int broadcast {1};
    // Set the broadcast option on the socket
    if (setsockopt(m_sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
    {
        std::cerr << "Error in setting Broadcast option." << std::endl;
        m_closeSocket();
        return false;
    }
    if (setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &M_RECV_TIMEOUT, sizeof(M_RECV_TIMEOUT)) < 0)
    {
        std::cerr << "Error in setting RECV TIMEOUT option." << std::endl;
        m_closeSocket();
        return false;
    }

    std::memset(&m_receiverAddr, 0, sizeof(m_receiverAddr));
    m_receiverAddr.sin_family = AF_INET;
    m_receiverAddr.sin_port = htobe16(m_port);

    m_receiverAddr.sin_addr.s_addr = inet_addr(m_ip.data());
    m_isConnectionActive = true;

    return true;
}

template <bool KnownKey>
void ServerConnection<KnownKey>::closeConnection()
{
    m_closeSocket();
    std::memset(&m_receiverAddr, 0, sizeof(m_receiverAddr));
}

template <bool KnownKey>
ServerConnection<KnownKey>::ServerConnection(ServerConnection &&serverConnection) noexcept
    : m_ip {serverConnection.m_ip}, m_port {serverConnection.m_port},
      m_sock {serverConnection.m_port}, m_receiverAddr {serverConnection.m_receiverAddr},
      m_isConnectionActive {serverConnection.m_isConnectionActive}
{
}

template <bool KnownKey>
ServerConnection<KnownKey> &
    ServerConnection<KnownKey>::operator=(ServerConnection<KnownKey> &&rhs) noexcept
{
    if (this != &rhs)
    {
        this->closeConnection();
        m_swap(*this, rhs);
    }
    return *this;
}

template <bool KnownKey>
ServerConnection<KnownKey>::~ServerConnection()
{
    this->closeConnection();
}

template class ServerConnection<true>;
template class ServerConnection<false>;
