#include "DataProcessing/SampleData.hpp"
#include "DataProcessing/TimingProcessing.hpp"
#include "SampleGroup.hpp"
#include "ServerConnection/ServerConnection.hpp"

#include <algorithm>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>

namespace
{
volatile sig_atomic_t g_continueRunning{true};
void exitHandler(int signal)
{
    (void)signal;
    g_continueRunning = false;
}

std::vector<std::byte> constructRandomVector(size_t size)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint8_t> uniform_dist(0, 255);
    std::vector<std::byte> randomized;
    randomized.reserve(size);
    for (size_t i{0}; i < size; ++i)
        randomized.push_back(static_cast<std::byte>(uniform_dist(gen)));
    return randomized;
}

bool saveToCsvFile(const std::string &fileName, const SampleGroup<long double> &sampleGroup)
{
    std::cout << "Saving current pass metrics to file." << std::endl;
    std::ofstream csvFilePass;
    csvFilePass.open(fileName);
    if (!csvFilePass)
    {
        std::cerr << "Could not open file: " << fileName << std::endl;
        return false;
    }
    constexpr std::string_view header{
        "Value, Mean, StdDev, StandardizedMean, StandardizedStdDev\n"};
    csvFilePass << header;
    for (unsigned value = 0; value < sampleGroup.size(); ++value)
    {
        SampleMetrics metrics = sampleGroup.localMetrics(value);
        SampleMetrics standardizedMetrics = sampleGroup.standardizeLocalMetrics(value);
        csvFilePass << static_cast<int>(static_cast<uint8_t>(value)) << ", " << std::setprecision(4)
                    << std::fixed << metrics.mean << ", " << metrics.stdDev << ", "
                    << standardizedMetrics.mean << ", " << standardizedMetrics.stdDev << "\n";
    }
    return true;
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
    constexpr unsigned NO_PASSES{128};

    SampleGroup<long double> sampleGroup{256, TRANSMISSION_COUNT * NO_PASSES};

    size_t id{0};
    for (size_t passNo{0}; passNo < NO_PASSES && g_continueRunning; passNo++)
    {
        for (unsigned value = 0; value < 256 && g_continueRunning; ++value)
        {
            std::vector<long double> sample;
            sample.reserve(TRANSMISSION_COUNT);
            for (size_t count{0}; count < TRANSMISSION_COUNT && g_continueRunning; ++count, ++id)
            {
                if (connection.connect())
                {
                    // Plaintext Construction
                    std::vector<std::byte> studyPlaintext = constructRandomVector(DATA_SIZE);
                    for (unsigned aesBlockNo = 0; aesBlockNo <= (DATA_SIZE - 1) / AES_BLOCK_SIZE;
                         ++aesBlockNo)
                    {
                        const unsigned fixedPoint = DATA_INDEX + aesBlockNo * AES_BLOCK_SIZE;
                        studyPlaintext[fixedPoint] = static_cast<std::byte>(value);
                    }
                    if (not g_continueRunning)
                    {
                        connection.closeConnection();
                        break;
                    }
                    auto result{connection.transmit(id, studyPlaintext)};
                    connection.closeConnection();
                    if (not result)
                    {
                        std::cerr << "Lost packet with id:\t" << id << " Loss rate: "
                                  << static_cast<float>(++lostPackages) /
                                         (static_cast<float>(id) + 1.0)
                                  << '\n';
                        count--;
                        continue;
                    }
                    sample.push_back(TimingProcessing::computeDT<long double>(
                        result->inbound_sec, result->inbound_nsec, result->outbound_sec,
                        result->outbound_nsec));
                }
            };
            if (g_continueRunning)
            {
                sampleGroup.insert(value, sample.begin(), sample.end());
                std::cout << "Pass no: " << passNo << " Value no: " << value << '\n';
            }
        }

        if (passNo % 8 == 7 or passNo == 0 or passNo + 1 == NO_PASSES or not g_continueRunning)
        {
            const std::string filename =
                std::string{argv[3]} + "_" + std::to_string(passNo) + ".csv";
            if (not saveToCsvFile(filename, sampleGroup))
                return EXIT_FAILURE;
        }
    }
    std::cout << "Exiting..." << std::endl;
    return 0;
}
