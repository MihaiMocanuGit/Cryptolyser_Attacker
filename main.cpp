#include "DataProcessing/SampleData.hpp"
#include "DataProcessing/TimingProcessing.hpp"
#include "ServerConnection/ServerConnection.hpp"

#include <algorithm>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iomanip>
#include <iostream>
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

    const std::string_view ip{argv[1]};
    const uint16_t port{static_cast<uint16_t>(std::stoi(argv[2]))};
    ServerConnection connection{ip, port};
    size_t lostPackages{0};

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

    constexpr unsigned DATA_SIZE{CONNECTION_DATA_MAX_SIZE};
    constexpr unsigned DATA_INDEX{5};
    constexpr unsigned TRANSMISSION_COUNT{516};
    constexpr unsigned AES_BLOCK_SIZE{128};
    std::vector<std::byte> dataTemplate(DATA_SIZE, std::byte(0));
    std::vector<SampleData<long double>> sampleData(256);
    constexpr std::string_view header{"Value, Mean, StdDev\n"};

    size_t id{0};
    for (size_t passNo{0}; passNo < 10 && g_continueRunning; passNo++)
    {
        for (unsigned value = 0; value < 256 && g_continueRunning; ++value)
        {
            for (int aesBlockNo = 0; aesBlockNo <= DATA_SIZE / AES_BLOCK_SIZE; ++aesBlockNo)
            {
                dataTemplate[DATA_INDEX + aesBlockNo * AES_BLOCK_SIZE] = std::byte(value);
            }

            std::vector<long double> sample;
            sample.reserve(TRANSMISSION_COUNT);
            for (size_t count{0}; count < TRANSMISSION_COUNT && g_continueRunning; ++count, ++id)
            {
                if (connection.connect())
                {
                    auto result{connection.transmit(id, dataTemplate)};
                    connection.closeConnection();
                    if (not result and g_continueRunning)
                    {
                        std::cerr << "Lost packet with id:\t" << id
                                  << " Loss rate: " << static_cast<float>(++lostPackages) / id
                                  << '\n';
                        count--;
                        continue;
                    }

                    sample.push_back(TimingProcessing::computeDT<long double>(
                        result->inbound_sec, result->inbound_nsec, result->outbound_sec,
                        result->outbound_nsec));
                }
            };
            // sampleData[value] = SampleData{std::move(sample)};
            sampleData[value].insert(sample.begin(), sample.end());
            std::cout << "Pass no: " << passNo << " Value no: " << value << '\n';
        }
        std::cout << "Saving current pass metrics to file." << std::endl;
        std::ofstream csvFilePass;
        csvFilePass.open(std::string{argv[3]} + std::to_string(passNo) + ".csv");
        if (!csvFilePass)
        {
            std::cerr << "Could not open file: " << argv[3] << std::endl;
            return EXIT_FAILURE;
        }
        csvFilePass << header;
        for (unsigned value = 0; value < sampleData.size(); ++value)
        {
            SampleMetrics metrics = sampleData[value].metrics();
            csvFilePass << static_cast<int>(static_cast<uint8_t>(value)) << ", "
                        << std::setprecision(3) << std::fixed << metrics.mean << ", "
                        << metrics.stdDev << "\n";
        }
    }
    std::cout << "Exiting..." << std::endl;
    return 0;
}
