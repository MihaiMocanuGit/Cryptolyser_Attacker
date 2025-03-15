#ifndef CRYPTOLYSER_ATTACKER_SERVERKEYCONNECTION_HPP
#define CRYPTOLYSER_ATTACKER_SERVERKEYCONNECTION_HPP

#include "Cryptolyser_Common/connection_data_types.h"

#include <array>
#include <cstdint>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <vector>

// For now, this is a raw copy paste from ServerConnection that is manually modified to connect to
// the Doppelganger instead of the Victim. It's used in this form to easily experiment with
// different configurations. In the end, this will be refactored into a nicer
// interface-implementation structure.
class ServerKeyConnection
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
    ServerKeyConnection(std::string_view ip, uint16_t port);
    ServerKeyConnection(ServerKeyConnection &&serverConnection) noexcept;
    ServerKeyConnection &operator=(ServerKeyConnection &&rhs) noexcept;
    ~ServerKeyConnection();

    ServerKeyConnection(const ServerKeyConnection &serverConnection) = delete;
    ServerKeyConnection &operator=(const ServerKeyConnection &rhs) = delete;

    bool connect();

    std::optional<connection_timing_t>
        transmit(uint32_t packet_id, const std::array<std::byte, PACKET_KEY_BYTE_SIZE> &key,
                 const std::vector<std::byte> &bytes);

    void closeConnection();
};

#endif // CRYPTOLYSER_ATTACKER_SERVERKEYCONNECTION_HPP
