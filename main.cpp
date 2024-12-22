#include "ServerConnection/ServerConnection.hpp"

#include <algorithm>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Incorrect program parameters: <IPv4> <PORT>" << std::endl;
        return -1;
    }
    const std::string_view ip = argv[1];
    const uint16_t port = std::stoi(argv[2]);

    ServerConnection connection{ip, port};
    std::cout << "Count, InboundSec, InboundNanoSec, OutboundSec, OutboundNanoSec\n";
    for (size_t count = 0;;)
    {
        for (size_t dataSize = 1; dataSize <= 500; ++dataSize, count++)
        {
            if (connection.connect())
            {
                std::vector<std::byte> data{dataSize};
                std::generate(data.begin(), data.end(),
                              []()
                              {
                                  static uint8_t value = 0;
                                  return static_cast<std::byte>(value++);
                              });
                auto result = connection.transmit(data);
                if (result)
                {
                    std::cout << count << ", " << dataSize << ", " << result->inbound_sec << ", "
                              << result->inbound_nsec << ", " << result->outbound_sec << ", "
                              << result->outbound_nsec << '\n';
                }
                connection.closeConnection();
            }
            else
            {
                std::cerr << "Could not connect." << std::endl;
            }
        }
    }
    return 0;
}
