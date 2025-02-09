#ifndef CRYPTOLYSER_ATTACKER_SERVERCONNECTION_HPP
#define CRYPTOLYSER_ATTACKER_SERVERCONNECTION_HPP

#include "Cryptolyser_Common/connection_data_types.h"

#include <cstdint>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <vector>

class ServerConnection
{
  private:
    const std::string_view m_ip;
    uint16_t m_port;
    int m_sock{-1};
    sockaddr_in m_receiverAddr{};
    bool m_isConnectionActive{false};
    static constexpr timeval M_RECV_TIMEOUT{/*.tv_sec*/ 1, /*.tv_usec*/ 0};

    void m_closeSocket();

  public:
    static const uint32_t DATA_MAX_SIZE;
    ServerConnection(std::string_view ip, uint16_t port);
    ServerConnection(const ServerConnection &serverConnection) = delete;
    ServerConnection &operator=(const ServerConnection &rhs) = delete;
    ~ServerConnection();

    bool connect();

    std::optional<connection_timing_t> transmit(uint32_t packet_id,
                                                const std::vector<std::byte> &bytes);

    void closeConnection();
};

#endif // CRYPTOLYSER_ATTACKER_SERVERCONNECTION_HPP
