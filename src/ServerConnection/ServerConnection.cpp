#include "ServerConnection.hpp"

#include "Cryptolyser_Common/connection_data_types.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>

constexpr uint32_t ServerConnection::DATA_MAX_SIZE{CONNECTION_DATA_MAX_SIZE};

void ServerConnection::m_closeSocket()
{
    close(m_sock);
    m_isConnectionActive = false;
}

ServerConnection::ServerConnection(std::string_view ip, uint16_t port) : m_ip{ip}, m_port{port} {}

bool ServerConnection::connect()
{
    m_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sock < 0)
    {
        std::cerr << "Error creating socket" << std::endl;
        return false;
    }

    int broadcast{1};
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

std::optional<connection_timing_t> ServerConnection::transmit(uint32_t packet_id,
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
    connection_packet_t packet{};
    packet.packet_id = htobe32(packet_id);
    packet.data_length = htobe32(bytes.size());
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
    responseTimingData.inbound_sec = be64toh(responseTimingData.inbound_sec);
    responseTimingData.inbound_nsec = be64toh(responseTimingData.inbound_nsec);
    responseTimingData.outbound_sec = be64toh(responseTimingData.outbound_sec);
    responseTimingData.outbound_nsec = be64toh(responseTimingData.outbound_nsec);
    return {responseTimingData};
}

void ServerConnection::closeConnection()
{
    m_closeSocket();
    std::memset(&m_receiverAddr, 0, sizeof(m_receiverAddr));
}

ServerConnection::~ServerConnection() { this->closeConnection(); }
