#include "ServerConnection/ServerConnection.hpp"

#include <algorithm>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iomanip>
#include <iostream>

void printStats(const std::chrono::time_point<std::chrono::high_resolution_clock> &timeStart,
                size_t lostPackages, size_t count)
{
    auto timeNow = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(timeNow - timeStart).count();

    std::ostringstream col1;
    col1 << count << " packages";
    constexpr unsigned width1{sizeof("1234567 packages.")};

    std::ostringstream col2;
    col2 << std::fixed << std::setprecision(2) << duration << " ms";
    constexpr unsigned width2{sizeof("123456789.12 ms")};

    std::ostringstream col3;
    col3 << std::fixed << std::setprecision(8) << (long double)lostPackages / count << " %";
    constexpr unsigned width3{sizeof("0.12345678 %")};

    std::cout << std::right << std::setw(width1) << col1.str() << " |";
    std::cout << std::right << std::setw(width2) << col2.str() << " |";
    std::cout << std::right << std::setw(width3) << col3.str() << " |" << std::endl;
}

namespace
{
volatile sig_atomic_t g_continueRunning{true};
void exitHandler(int signal)
{
    (void)signal;
    g_continueRunning = false;
}
} // namespace

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Incorrect program parameters: <IPv4> <PORT> <CSV_FILE_PATH>" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Starting..." << std::endl;
    auto timeStart = std::chrono::high_resolution_clock::now();

    const std::string_view ip{argv[1]};
    const uint16_t port{static_cast<uint16_t>(std::stoi(argv[2]))};
    ServerConnection connection{ip, port};
    size_t lostPackages{0};

    std::ofstream csvFile;
    csvFile.open(argv[3]);
    if (!csvFile)
    {
        std::cerr << "Could not open file: " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    const std::string_view header{
        "Id, Size, InboundSec, InboundNanoSec, OutboundSec, OutboundNanoSec\n"};
    csvFile << header;

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

    for (size_t dataSize{1}, id{1}; g_continueRunning;
         dataSize = dataSize % ServerConnection::DATA_MAX_SIZE + 1, id++)
    {
        if (connection.connect())
        {
            std::vector<std::byte> data{dataSize};
            std::generate(data.begin(), data.end(),
                          []()
                          {
                              static uint8_t value{0};
                              return static_cast<std::byte>(value++);
                          });
            auto result{connection.transmit(id, data)};
            if (result)
            {
                const std::string outputRow{std::to_string(id) + ", " + std::to_string(dataSize) + ", " +
                                            std::to_string(result->inbound_sec) + ", " +
                                            std::to_string(result->inbound_nsec) + ", " +
                                            std::to_string(result->outbound_sec) + ", " +
                                            std::to_string(result->outbound_nsec) + '\n'};
                csvFile << outputRow;
            }
            else if (g_continueRunning)
            {
                ++lostPackages;
                const std::string outputRow{std::to_string(id) + ", " + std::to_string(dataSize) + ", -1, -1, -1, -1\n"};
                csvFile << outputRow;
            }
            connection.closeConnection();

            if (id % ServerConnection::DATA_MAX_SIZE == 0)
                printStats(timeStart, lostPackages, id);
        }
        else
        {
            std::cerr << "Could not connect." << std::endl;
        }
    }
    std::cout << "Exiting..." << std::endl;
    return 0;
}
