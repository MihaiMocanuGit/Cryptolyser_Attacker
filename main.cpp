#include <iostream>
#include <algorithm>
#include "ServerConnection/ServerConnection.hpp"

int main(int argc, char **argv)
{
    ServerConnection connection{"192.168.1.147", 8081};
    for (size_t count = 0;;)
    {
        for (size_t i = 1; i <= 500; ++i, count++)
        {
            if (connection.connect())
            {
                std::vector<std::byte> data{i};
                std::generate(data.begin(), data.end(), [&]()
                {
                    static uint8_t value = 0;
                    return static_cast<std::byte>(value++);
                });
                auto result = connection.transmit(data);
                std::cout << count << ":\tdelta:\t" << result->outbound - result->inbound << "\tdata size:\t" << i
                          << '\n';
                connection.closeConnection();
            } else
            {
                std::cerr << "Could not connect." << std::endl;
            }
        }
    }
    return 0;
}
