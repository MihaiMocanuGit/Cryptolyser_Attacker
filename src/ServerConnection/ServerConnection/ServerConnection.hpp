#ifndef CRYPTOLYSER_ATTACKER_SERVERCONNECTION_HPP
#define CRYPTOLYSER_ATTACKER_SERVERCONNECTION_HPP

#include "Cryptolyser_Common/connection_data_types.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <vector>

template <bool KnownKey>
class ServerConnection
{
  private:
    std::string_view m_ip;
    uint16_t m_port;
    int m_sock{-1};
    sockaddr_in m_receiverAddr{};
    bool m_isConnectionActive{false};
    static constexpr timeval M_RECV_TIMEOUT{/*.tv_sec*/ 1, /*.tv_usec*/ 0};

    void m_closeSocket();
    static void m_swap(ServerConnection &server1, ServerConnection &server2);

  public:
    static const uint32_t DATA_MAX_SIZE;
    ServerConnection(std::string_view ip, uint16_t port);
    ServerConnection(ServerConnection &&serverConnection) noexcept;
    ServerConnection &operator=(ServerConnection &&rhs) noexcept;
    ~ServerConnection();

    ServerConnection(const ServerConnection &serverConnection) = delete;
    ServerConnection &operator=(const ServerConnection &rhs) = delete;

    bool connect();

    template <bool T = KnownKey>
    typename std::enable_if<T, std::optional<connection_timing_t>>::type
        transmit(uint32_t packet_id, const std::array<std::byte, AES_BLOCK_BYTE_SIZE> &key,
                 const std::vector<std::byte> &bytes);

    template <bool T = not KnownKey>
    typename std::enable_if<T, std::optional<connection_timing_t>>::type
        transmit(uint32_t packet_id, const std::vector<std::byte> &bytes);

    void closeConnection();
};

template <bool KnownKey>
template <bool T>
typename std::enable_if<T, std::optional<connection_timing_t>>::type
    ServerConnection<KnownKey>::transmit(uint32_t packet_id, const std::array<std::byte, 16> &key,
                                         const std::vector<std::byte> &bytes)
{
    if (not m_isConnectionActive)
        return {};
    if (CONNECTION_DATA_MAX_SIZE < bytes.size())
    {
        std::cerr << "Error, too many bytes for a single packet." << std::endl;
        m_closeSocket();
        return {};
    }
    connection_key_packet_t packet{};
    packet.packet_id = htobe32(packet_id);
    packet.data_length = htobe32(bytes.size());
    std::memcpy(packet.key, key.data(), AES_BLOCK_BYTE_SIZE);
    if (not bytes.empty())
        std::memcpy(packet.byte_data, bytes.data(), bytes.size());
    if (sendto(m_sock, &packet, sizeof(packet), 0,
               reinterpret_cast<struct sockaddr *>(&m_receiverAddr), sizeof(m_receiverAddr)) < 0)
    {
        std::cerr << "Error in sending data packet." << std::endl;
        m_closeSocket();
        return {};
    }

    connection_timing_t responseTimingData{};
    socklen_t len{sizeof(struct sockaddr_in)};
    if (recvfrom(m_sock, &responseTimingData, sizeof(responseTimingData), 0,
                 reinterpret_cast<struct sockaddr *>(&m_receiverAddr), &len) < 0)
    {
        std::cerr << "Error in receiving message" << std::endl;
        if (errno == EWOULDBLOCK)
        {
            std::cerr << "Reason: timeout." << std::endl;
        }
        m_closeSocket();
        return {};
    }
    responseTimingData.packet_id = be32toh(responseTimingData.packet_id);
    responseTimingData.inbound_t1 = be64toh(responseTimingData.inbound_t1);
    responseTimingData.inbound_t2 = be64toh(responseTimingData.inbound_t2);
    responseTimingData.outbound_t1 = be64toh(responseTimingData.outbound_t1);
    responseTimingData.outbound_t2 = be64toh(responseTimingData.outbound_t2);
    return {responseTimingData};
}

template <bool KnownKey>
template <bool T>
typename std::enable_if<T, std::optional<connection_timing_t>>::type
    ServerConnection<KnownKey>::transmit(uint32_t packet_id, const std::vector<std::byte> &bytes)
{
    if (not m_isConnectionActive)
        return {};
    if (CONNECTION_DATA_MAX_SIZE < bytes.size())
    {
        std::cerr << "Error, too many bytes for a single packet." << std::endl;
        m_closeSocket();
        return {};
    }
    connection_packet_t packet{};
    packet.packet_id = htobe32(packet_id);
    packet.data_length = htobe32(bytes.size());
    if (not bytes.empty())
        std::memcpy(packet.byte_data, bytes.data(), bytes.size());
    if (sendto(m_sock, &packet, sizeof(packet), 0,
               reinterpret_cast<struct sockaddr *>(&m_receiverAddr), sizeof(m_receiverAddr)) < 0)
    {
        std::cerr << "Error in sending data packet." << std::endl;
        m_closeSocket();
        return {};
    }

    connection_timing_t responseTimingData{};
    socklen_t len{sizeof(struct sockaddr_in)};
    if (recvfrom(m_sock, &responseTimingData, sizeof(responseTimingData), 0,
                 reinterpret_cast<struct sockaddr *>(&m_receiverAddr), &len) < 0)
    {
        std::cerr << "Error in receiving message" << std::endl;
        if (errno == EWOULDBLOCK)
        {
            std::cerr << "Reason: timeout." << std::endl;
        }
        m_closeSocket();
        return {};
    }
    responseTimingData.packet_id = be32toh(responseTimingData.packet_id);
    responseTimingData.inbound_t1 = be64toh(responseTimingData.inbound_t1);
    responseTimingData.inbound_t2 = be64toh(responseTimingData.inbound_t2);
    responseTimingData.outbound_t1 = be64toh(responseTimingData.outbound_t1);
    responseTimingData.outbound_t2 = be64toh(responseTimingData.outbound_t2);
    return {responseTimingData};
}

#endif // CRYPTOLYSER_ATTACKER_SERVERCONNECTION_HPP
