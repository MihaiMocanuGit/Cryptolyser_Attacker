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

bool saveMetrics(const std::string &fileName, const SampleGroup<long double> &sampleGroup)
{
    std::ofstream out;
    out.open(fileName);
    if (!out)
    {
        std::cerr << "Could not open file: " << fileName << std::endl;
        return false;
    }
    constexpr std::string_view header{
        "Value, Mean, StdDev, StandardizedMean, StandardizedStdDev\n"};
    out << header;
    for (unsigned value = 0; value < sampleGroup.size(); ++value)
    {
        SampleMetrics metrics = sampleGroup.localMetrics(value);
        SampleMetrics standardizedMetrics = sampleGroup.standardizeLocalMetrics(value);
        out << static_cast<int>(static_cast<uint8_t>(value)) << ", " << std::setprecision(4)
            << std::fixed << metrics.mean << ", " << metrics.stdDev << ", "
            << standardizedMetrics.mean << ", " << standardizedMetrics.stdDev << "\n";
    }
    out.close();
    return true;
}

bool saveData(const std::string &fileName, const SampleGroup<long double> &sampleGroup,
              size_t passTransmissionCount)
{
    std::ofstream out;
    out.open(fileName);
    if (!out)
    {
        std::cerr << "Could not open file: " << fileName << std::endl;
        return false;
    }

    SampleMetrics<long double> globalMetrics = sampleGroup.globalMetrics();
    out << "Global size: " << globalMetrics.size << " mean: " << globalMetrics.mean
        << " stdDev: " << globalMetrics.stdDev;
    for (unsigned valueIndex = 0; valueIndex < sampleGroup.size(); ++valueIndex)
    {
        const SampleData<long double> &currentSample{sampleGroup[valueIndex]};
        SampleMetrics<long double> currentMetrics = currentSample.metrics();
        out << "\n\nValues for " << valueIndex << ":\n";
        out << "Local size: " << currentMetrics.size << " mean: " << currentMetrics.mean
            << " stdDev: " << currentMetrics.stdDev;
        for (size_t i{0}; i < currentSample.data().size(); ++i)
        {
            if (i % passTransmissionCount == 0)
                out << '\n';
            out << currentSample.data()[i];
            if ((i + 1) % passTransmissionCount != 0)
                out << ", ";
            else
                out << ';';
        }
    }
    out.close();
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
            std::vector<std::byte> studyPlaintext;
            bool stopAndTryAgain = false;
            size_t valueRetries{0}, networkRetries{0};
            for (size_t count{0}; count < TRANSMISSION_COUNT && g_continueRunning; ++count, ++id)
            {
                if (connection.connect())
                {
                    // Plaintext Construction
                    if (not stopAndTryAgain)
                    {
                        studyPlaintext = constructRandomVector(DATA_SIZE);
                        for (unsigned aesBlockNo = 0;
                             aesBlockNo <= (DATA_SIZE - 1) / AES_BLOCK_SIZE; ++aesBlockNo)
                        {
                            const unsigned fixedPoint = DATA_INDEX + aesBlockNo * AES_BLOCK_SIZE;
                            studyPlaintext[fixedPoint] = static_cast<std::byte>(value);
                        }
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
                                  << static_cast<double>(++lostPackages) /
                                         (static_cast<double>(id + valueRetries + networkRetries) +
                                          1.0)
                                  << std::endl;
                        networkRetries++;
                        count--;
                        id--;
                        stopAndTryAgain = true;
                        continue;
                    }
                    long double timing{TimingProcessing::computeDT<long double>(
                        result->inbound_sec, result->inbound_nsec, result->outbound_sec,
                        result->outbound_nsec)};
                    // TODO: Make a proper testing criteria, using the mean and variance
                    if (timing > 500 and timing < 17000.0)
                        sample.push_back(timing);
                    else
                    {
                        // TODO: If the same packet takes more than M tries, do accept it
                        valueRetries++;
                        count--;
                        id--;
                        stopAndTryAgain = true;
                        continue;
                    }
                    stopAndTryAgain = false;
                }
            };
            if (g_continueRunning)
            {
                sampleGroup.insert(value, sample.begin(), sample.end());
                std::cout << "Pass no: " << passNo << " Value no: " << value
                          << " Retries: " << valueRetries << '\n';
            }
        }

        std::cout << "\nSaving pass " << passNo << " metrics to files.\n" << std::endl;
        const std::string metricsFilename =
            std::string{argv[3]} + '_' + std::to_string(passNo) + "_metrics.csv";
        if (not saveMetrics(metricsFilename, sampleGroup))
            return EXIT_FAILURE;

        const std::string valuesFilename =
            std::string{argv[3]} + '_' + std::to_string(passNo) + "_data.txt";
        if (not saveData(valuesFilename, sampleGroup, TRANSMISSION_COUNT))
            return EXIT_FAILURE;
    }

    std::cout << "Exiting..." << std::endl;
    return 0;
}
