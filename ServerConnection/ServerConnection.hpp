#ifndef CRYPTOLYSER_ATTACKER_SERVERCONNECTION_HPP
#define CRYPTOLYSER_ATTACKER_SERVERCONNECTION_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <netinet/in.h>
#include <optional>

class ServerConnection
{
private:
    std::string m_ip;
    uint16_t m_port;
    int m_sock{-1};
    sockaddr_in m_receiverAddr{};
    bool m_isConnectionActive{false};
    static constexpr timeval M_RECV_TIMEOUT = {.tv_sec = 1, .tv_usec = 0};

#pragma pack(1)
    struct Packet
    {
        static constexpr size_t MAX_DATA_SIZE = 500;
        uint64_t dataLength;
        uint8_t byteData[MAX_DATA_SIZE];
    };
#pragma pack(0)

    void m_closeSocket();
public:
#pragma pack(1)
    struct TimingData
    {
        uint64_t inbound;
        uint64_t outbound;
    };
#pragma pack(0)

    ServerConnection(std::string ip, uint16_t port) noexcept;
    ServerConnection(const ServerConnection &serverConnection) = delete;
    ServerConnection &operator=(const ServerConnection &rhs) = delete;
    ~ServerConnection();

    bool connect();

    std::optional<ServerConnection::TimingData> transmit(const std::vector<std::byte> &bytes);

    void closeConnection();
};

#endif //CRYPTOLYSER_ATTACKER_SERVERCONNECTION_HPP
