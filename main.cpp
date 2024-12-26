#include "ServerConnection/ServerConnection.hpp"

#include <algorithm>
#include <csignal>
#include <fstream>
#include <iostream>

namespace
{
volatile sig_atomic_t g_continueRunning = true;
void exitHandler(int signal) { g_continueRunning = false; }
} // namespace

int main(int argc, char **argv)
{
    constexpr bool PRINT_TO_STD = false;
    if (argc != 4)
    {
        std::cerr << "Incorrect program parameters: <IPv4> <PORT> <CSV_FILE_PATH>" << std::endl;
        return -1;
    }

    const std::string_view ip{argv[1]};
    const uint16_t port{static_cast<uint16_t>(std::stoi(argv[2]))};
    std::ofstream csvFile;
    csvFile.open(argv[3]);

    ServerConnection connection{ip, port};
    const std::string header{
        "Count, Size, InboundSec, InboundNanoSec, OutboundSec, OutboundNanoSec\n"};
    if constexpr (PRINT_TO_STD)
        std::cout << header;
    csvFile << header;
    size_t lostPackages = 0;

    struct sigaction sigactionExit{};
    sigactionExit.sa_handler = exitHandler;
    sigemptyset(&sigactionExit.sa_mask);
    sigactionExit.sa_flags = SA_RESTART;
    for (auto sig : {SIGTERM, SIGINT, SIGQUIT})
    {
        if (sigaction(sig, &sigactionExit, nullptr))
        {
            std::perror("Error setting signal handler\n");
            return EXIT_FAILURE;
        }
    }

    for (size_t dataSize{1}, count{0}; g_continueRunning; dataSize = dataSize % 500 + 1, count++)
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
            auto result{connection.transmit(data)};
            if (result)
            {
                const std::string outputRow{std::to_string(count) + ", " +
                                            std::to_string(dataSize) + ", " +
                                            std::to_string(result->inbound_sec) + ", " +
                                            std::to_string(result->inbound_nsec) + ", " +
                                            std::to_string(result->outbound_sec) + ", " +
                                            std::to_string(result->outbound_nsec) + '\n'};
                if constexpr (PRINT_TO_STD)
                    std::cout << outputRow;
                csvFile << outputRow;
            }
            else if (g_continueRunning)
            {
                std::cout << "Loss ratio: "
                          << static_cast<long double>(++lostPackages) / (count + 1) << std::endl;
                const std::string outputRow{std::to_string(count) + ", " +
                                            std::to_string(dataSize) + ", -1, -1, -1, -1\n"};
                csvFile << outputRow;
            }
            connection.closeConnection();
        }
        else
        {
            std::cerr << "Could not connect." << std::endl;
        }
    }
    return 0;
}
