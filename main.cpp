#include "DataProcessing/SampleData.hpp"
#include "DataProcessing/TimingProcessing.hpp"
#include "SampleGroup.hpp"
#include "ServerConnection/ServerConnection.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <csignal>
#include <filesystem>
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

template <size_t GROUPS_COUNT>
bool saveMetrics(size_t currentCount, const std::string &saveFilePath,
                 const std::array<SampleGroup<double>, GROUPS_COUNT> &sampleGroups)
{
    const std::string root{saveFilePath + "/" + std::to_string(currentCount)};
    std::filesystem::create_directory(root);
    for (unsigned i{0}; i < GROUPS_COUNT; ++i)
    {
        const std::string filepath{root + "/" + std::to_string(i) + ".csv"};
        std::ofstream out;
        out.open(filepath);
        if (!out)
        {
            std::cerr << "Could not create file: " << filepath << std::endl;
            return false;
        }
        constexpr std::string_view header{
            "Value, Mean, StdDev, Size, StandardizedMean, StandardizedStdDev\n"};
        out << header;
        for (unsigned byteValue = 0; byteValue < sampleGroups[i].size(); ++byteValue)
        {
            SampleMetrics metrics = sampleGroups[i].localMetrics(byteValue);
            SampleMetrics standardizedMetrics = sampleGroups[i].standardizeLocalMetrics(byteValue);
            out << static_cast<int>(static_cast<uint8_t>(byteValue)) << ", " //
                << std::setprecision(8) << std::fixed                        //
                << metrics.mean << ", "                                      //
                << metrics.stdDev << ", "                                    //
                << metrics.size << ", "                                      //
                << standardizedMetrics.mean << ", "                          //
                << standardizedMetrics.stdDev << "\n";                       //
        }
        out.close();
    }

    return true;
}

template <size_t GROUPS_COUNT>
bool saveRaw(const std::string &saveFilePath,
             const std::array<SampleGroup<double>, GROUPS_COUNT> &sampleGroups)
{
    std::filesystem::create_directory(saveFilePath + "/Raw");
    for (unsigned byteBlock{0}; byteBlock < GROUPS_COUNT; ++byteBlock)
    {
        std::string currentLevelPath{saveFilePath + "/Raw/Byte_" + std::to_string(byteBlock)};
        std::filesystem::create_directory(currentLevelPath);
        const auto &sampleGroup{sampleGroups[byteBlock]};
        for (unsigned value{0}; value < sampleGroup.size(); ++value)
        {
            std::ofstream out;
            out.open(currentLevelPath + "/Value_" + std::to_string(value) + ".csv");
            if (!out)
            {
                std::cerr << "Could not create file: "
                          << currentLevelPath + "/Value_" + std::to_string(value) << std::endl;
                return false;
            }
            const auto &data{sampleGroup[value].data()};
            out << "INDICES, VALUES, SIZE\n";
            if (data.size() > 0)
            {
                out << 0 << ", " << data[0] << ", " << data.size() << '\n';
            }
            for (size_t i{1}; i < data.size(); ++i)
            {
                out << i << ", " << data[i] << ",\n";
            }
        }
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
    const std::string saveFilepath{argv[3]};
    std::cout << "Creating directory: " + saveFilepath << std::endl;
    std::filesystem::create_directory(saveFilepath);

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

    constexpr unsigned AES_BLOCK_SIZE{16};
    constexpr unsigned SAMPLE_GROUP_SIZE{256};

    constexpr unsigned DESIRED_SIZE_OF_SAMPLE{4048 * 3};
    constexpr unsigned DATA_SIZE{512};

    constexpr double TIMING_LB{100.0};
    constexpr double TIMING_UB{12000.0};

    constexpr unsigned PRINT_FREQ{200'000};
    constexpr unsigned SAVE_FREQ{1000'000};

    std::array<SampleGroup<double>, AES_BLOCK_SIZE> sampleGroups;
    sampleGroups.fill({SAMPLE_GROUP_SIZE, DESIRED_SIZE_OF_SAMPLE});

    constexpr size_t APPROXIMATE_TOTAL_COUNT{AES_BLOCK_SIZE * SAMPLE_GROUP_SIZE *
                                             DESIRED_SIZE_OF_SAMPLE};
    size_t outliersCountUB{0};
    double outliersMeanUB{0};
    double outliersMinUB{std::numeric_limits<double>::max()};

    size_t outliersCountLB{0};
    double outliersMeanLB{0};
    double outliersMaxLB{std::numeric_limits<double>::min()};

    size_t actualTotalCount =
        APPROXIMATE_TOTAL_COUNT + lostPackages + outliersCountLB + outliersCountUB;

    timespec startTime{};
    clock_gettime(CLOCK_MONOTONIC, &startTime);

    timespec prevTime{0, 0};
    size_t prevPacketCount{0};

    std::cout << "Starting the study..." << std::endl;
    if (connection.connect())
    {
        size_t count{0};
        for (count = 0; count < actualTotalCount and g_continueRunning; ++count)
        {

            std::vector<std::byte> studyPlaintext = constructRandomVector(DATA_SIZE);
            const auto result{connection.transmit(count, studyPlaintext)};
            if (not result)
            {
                std::cerr << "Lost packet with id:\t" << count << " Loss rate: "
                          << static_cast<double>(++lostPackages) / (static_cast<double>(count + 1))
                          << std::endl;
                // restart the connection
                connection.closeConnection();
                connection.connect();
                continue;
            }
            const double timing{TimingProcessing::computeDT<double>(
                result->inbound_t1, result->inbound_t2, result->outbound_t1, result->outbound_t2)};
            // TODO: Make a proper testing criteria, using the mean and variance
            if (timing < TIMING_LB)
            {
                outliersMeanLB = (outliersMeanLB * static_cast<double>(outliersCountLB) + timing) /
                                 static_cast<double>(outliersCountLB + 1);
                outliersCountLB++;
                outliersMaxLB = std::max(outliersMaxLB, timing);
            }
            else if (timing > TIMING_UB)
            {
                outliersMeanUB = (outliersMeanUB * static_cast<double>(outliersCountUB) + timing) /
                                 static_cast<double>(outliersCountUB + 1);
                outliersCountUB++;
                outliersMinUB = std::min(outliersMinUB, timing);
            }
            else
            {
                for (unsigned byteIndex{0}; byteIndex < AES_BLOCK_SIZE and byteIndex < DATA_SIZE;
                     ++byteIndex)
                    sampleGroups[byteIndex].insert(static_cast<size_t>(studyPlaintext[byteIndex]),
                                                   timing);
            }

            actualTotalCount =
                APPROXIMATE_TOTAL_COUNT + lostPackages + outliersCountLB + outliersCountUB;
            if ((count != 0 and count % PRINT_FREQ == 0) or count + 1 == actualTotalCount or
                not g_continueRunning)
            {

                const double completionPercent{static_cast<double>(count + 1) /
                                               static_cast<double>(actualTotalCount) * 100.0};

                timespec elapsedTime{};
                clock_gettime(CLOCK_MONOTONIC, &elapsedTime);
                elapsedTime.tv_sec -= startTime.tv_sec;

                // Estimated time until completion in minutes
                const double ETA = (100.0 - completionPercent) *
                                   static_cast<double>(elapsedTime.tv_sec) /
                                   (completionPercent * 60.0);

                const double rate = static_cast<double>(count - prevPacketCount) /
                                    static_cast<double>(elapsedTime.tv_sec - prevTime.tv_sec);
                prevPacketCount = count;
                prevTime = elapsedTime;
                // TODO: fixed width columns
                std::cout << "Stats:\n";
                std::cout << "\tETA: " << ETA << " minutes"
                          << "\t Progress: " << count + 1 << '/' << actualTotalCount << " ("
                          << completionPercent << "%)"
                          << "\t Study Rate: " << rate << " packets/second";
                std::cout << '\n';

                std::cout << "Sample Group:" << '\n';
                std::cout << std::fixed << std::setprecision(3)
                          << "\tSize: " << sampleGroups[0].globalMetrics().size
                          << "\tMean: " << sampleGroups[0].globalMetrics().mean;
                std::cout << '\n';

                std::cout << "LB Outliers:\n";
                std::cout << "\tCount: " << outliersCountLB << "\t Mean: " << outliersMeanLB
                          << "\t Max: ";
                if (outliersCountLB)
                    std::cout << outliersMaxLB;
                std::cout << '\n';

                std::cout << "UB Outliers:\n";
                std::cout << "\tCount: " << outliersCountUB << "\t Mean: " << outliersMeanUB
                          << "\t Min: ";
                if (outliersCountUB)
                    std::cout << outliersMinUB;
                std::cout << "\n\n";
            }

            if ((count != 0 and count % SAVE_FREQ == 0) or count + 1 == actualTotalCount)
            {
                std::cout << "Writing Metrics File: " << count << "\n\n";
                if (not saveMetrics(count, saveFilepath, sampleGroups))
                    return EXIT_FAILURE;
            }
        }
        connection.closeConnection();

        if (not g_continueRunning)
        {
            std::cout << "Writing Metrics File: " << count << "\n\n";
            if (not saveMetrics(count, saveFilepath, sampleGroups))
                return EXIT_FAILURE;
        }
    }

    std::cout << "Saving Raw Data File:" << std::endl;
    if (not saveRaw(saveFilepath, sampleGroups))
        return EXIT_FAILURE;

    std::cout << "Exiting..." << std::endl;
    return 0;
}
