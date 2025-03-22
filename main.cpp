#include "DataProcessing/Samples/SampleGroup.hpp"
#include "DataProcessing/Timings/TimingProcessing.hpp"
#include "ServerConnection/ServerConnection.hpp"
#include "Study/Gatherer/Gatherer.hpp"
#include "Study/Study.hpp"
#include "Study/TimingData/TimingData.hpp"

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
    static int count = 0;
    (void)signal;
    if (count > 0)
    {
        std::cout << "Forced quit." << std::endl;
        exit(EXIT_SUCCESS);
    }
    g_continueRunning = false;
    count++;
}
} // namespace

namespace Experimental
{

void studyRun(const std::filesystem::path &saveFolderPath,
              ServerConnection<false> &connectionKeyless)
{
    constexpr size_t DESIRED_COUNT = 32 * 1024 * 1024;
    constexpr size_t LOG_FREQ = 128 * 1024;
    constexpr size_t SAVE_FREQ = 3 * 1024 * 1024;

    constexpr size_t DATA_SIZE = AES_BLOCK_BYTE_SIZE;
    constexpr size_t RESERVE = DESIRED_COUNT;

    TimingData<false> dataKeyless{DATA_SIZE, RESERVE};
    Gatherer<false> gathererKeyless{std::move(connectionKeyless), std::move(dataKeyless)};
    Study<false> studyKeyless{std::move(gathererKeyless), g_continueRunning, saveFolderPath};

    std::cout << "Started calibration..." << std::endl;
    auto [lb, ub] = studyKeyless.calibrateBounds(1'000'000, 0, 0.0015);
    std::cout << "Computed bounds: [" << lb << " " << ub << "]" << std::endl;

    std::cout << "Started run..." << std::endl;
    studyKeyless.run(DESIRED_COUNT, LOG_FREQ, SAVE_FREQ, lb, ub);
    std::cout << "Finished." << std::endl;

    gathererKeyless = studyKeyless.release();
    connectionKeyless = std::move(gathererKeyless.release().connection);
}

void studyRun(const std::filesystem::path &saveFolderPath, ServerConnection<true> &connectionKey,
              const std::array<std::byte, 16> &key)

{
    constexpr size_t DESIRED_COUNT = 32 * 1024 * 1024;
    constexpr size_t LOG_FREQ = 128 * 1024;
    constexpr size_t SAVE_FREQ = 3 * 1024 * 1024;

    constexpr size_t DATA_SIZE = AES_BLOCK_BYTE_SIZE;
    constexpr size_t RESERVE = DESIRED_COUNT;

    TimingData<true> dataKey{DATA_SIZE, RESERVE, key};
    Gatherer<true> gathererKey{std::move(connectionKey), std::move(dataKey)};
    Study<true> studyKey{std::move(gathererKey), g_continueRunning, saveFolderPath};

    std::cout << "Started calibration..." << std::endl;
    auto [lb, ub] = studyKey.calibrateBounds(1'000'000, 0, 0.0015);
    std::cout << "Computed bounds: [" << lb << " " << ub << "]" << std::endl;

    std::cout << "Started run..." << std::endl;
    studyKey.run(DESIRED_COUNT, LOG_FREQ, SAVE_FREQ, lb, ub);
    std::cout << "Finished." << std::endl;

    gathererKey = std::move(studyKey.release());
    connectionKey = std::move(gathererKey.release().connection);
}

void convert(const std::filesystem::path &saveResultPath, const std::filesystem::path &studyPath1,
             const std::array<std::byte, PACKET_KEY_BYTE_SIZE> &key1,
             const std::filesystem::path &studyPath2,
             const std::array<std::byte, PACKET_KEY_BYTE_SIZE> &key2)
{
    TimingData<true> original1(400, 2048 * 2048, key1);
    SaveLoad::loadRawFromTimingData(studyPath1 / "Raw", original1);

    TimingData<true> original2(400, 2048 * 2048, key2);
    SaveLoad::loadRawFromTimingData(studyPath2 / "Raw", original2);

    for (unsigned byte{0}; byte < AES_BLOCK_BYTE_SIZE; ++byte)
    {
        if (key1[byte] != key2[byte])
        {
            auto convertXor = [byte](const std::filesystem::path &saveResultPath,
                                     const TimingData<true> &original,
                                     const std::array<std::byte, PACKET_KEY_BYTE_SIZE> &key)
            {
                SampleGroup<double> group(256);
                for (unsigned byteValue{0}; byteValue < 256; ++byteValue)
                {
                    std::byte newIndex{static_cast<std::byte>(byteValue) ^ key[byte]};
                    double timingValue{
                        original.blockTimings[byte].standardizeLocalMetrics(byteValue).mean};
                    group.insert(static_cast<size_t>(newIndex), timingValue);
                }
                SaveLoad::saveMetricsFromSampleGroup(saveResultPath, group);
            };
            convertXor(saveResultPath / std::to_string(byte) / "Key1.csv", original1, key1);
            convertXor(saveResultPath / std::to_string(byte) / "Key2.csv", original2, key2);
        }
    }
};

void correlate(const std::filesystem::path &saveResultPath, const std::filesystem::path &studyPath)
{
    TimingData<false> u{400, 2048 * 2048};
    SaveLoad::loadRawFromTimingData(studyPath / "u" / "Raw", u);

    TimingData<false> t{400, 2048 * 2048};
    SaveLoad::loadRawFromTimingData(studyPath / "t" / "Raw", t);

    TimingData<false> result(400, 4);

    for (unsigned byte{0}; byte < AES_BLOCK_BYTE_SIZE; ++byte)
    {
        for (unsigned i{0}; i < 256; ++i)
        {
            long double correlation_i{0.0};
            for (unsigned j{0}; j < 256; ++j)
            {
                long double t_j{t.blockTimings[0].standardizeLocalMetrics(j).mean};
                long double u_i_j{u.blockTimings[0].standardizeLocalMetrics(i ^ j).mean};
                long double value{t_j * u_i_j};

                correlation_i += value;
            }
            result.blockTimings[byte].insert(i, correlation_i);
        }
    }
    SaveLoad::saveMetricsFromTimingData(saveResultPath, result);
    //    SaveLoad::saveMetricsFromSampleGroup(saveResultPath / "Byte_0/metrics.csv", result);
}

} // namespace Experimental

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Incorrect program parameters: <IPv4> <PORT> <SAVE_FOLDER_PATH>" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Starting..." << std::endl;
    const std::filesystem::path saveFolderPath{argv[3]};

    const std::string_view ip{argv[1]};
    const uint16_t port{static_cast<uint16_t>(std::stoi(argv[2]))};

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

    // ServerConnection<true> connectionKey(ip, port);
    // ServerConnection<false> connectionKeyless(ip, port);

    auto key1 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
    auto key2 = std::bit_cast<std::array<std::byte, 16>>(std::to_array<uint8_t>(
        {255, 254, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241}));
    auto key3 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 7}));
    auto key4 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));
    auto key5 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0, 0, 0, 0, 0, 0, 0, 0, 15, 15, 15, 15, 15, 15, 15, 15}));
    auto key6 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15}));

    //    Experimental::studyRun(saveFolderPath / "Key1_response/t", connectionKeyless);
    //    Experimental::studyRun(saveFolderPath / "Key1_response/u", connectionKeyless);
    //    {
    //        TimingData<false> combinedData{400, 2048 * 100};
    //        SaveLoad::loadRawFromTimingData(saveFolderPath / "Key1_response/t/Raw", combinedData);
    //        SaveLoad::loadRawFromTimingData(saveFolderPath / "Key1_response/u/Raw", combinedData);
    //        SaveLoad::saveMetricsFromTimingData(saveFolderPath / "Key1_response/Combined",
    //        combinedData);
    //    }
    //    Experimental::correlate(saveFolderPath / "Key1_response/Correlate",
    //                            saveFolderPath / "Key1_response");

    //    Experimental::studyRun(saveFolderPath / "Key2_response/t", connectionKey, key2);
    //    Experimental::studyRun(saveFolderPath / "Key2_response/u", connectionKey, key2);
    //    {
    //        TimingData<true> combinedData{400, 2048 * 100, key2};
    //        SaveLoad::loadRawFromTimingData(saveFolderPath / "Key2_response/t/Raw", combinedData);
    //        SaveLoad::loadRawFromTimingData(saveFolderPath / "Key2_response/u/Raw", combinedData);
    //        SaveLoad::saveMetricsFromTimingData(saveFolderPath / "Key2_response/Combined",
    //                                            combinedData);
    //    }
    //    Experimental::correlate(saveFolderPath / "Key2_response/Correlate",
    //                            saveFolderPath / "Key2_response");

    //    Experimental::studyRun(saveFolderPath / "Key3_response/t", connectionKey, key3);
    //    Experimental::studyRun(saveFolderPath / "Key3_response/u", connectionKey, key3);
    //    {
    //        TimingData<true> combinedData{400, 2048 * 100, key3};
    //        SaveLoad::loadRawFromTimingData(saveFolderPath / "Key3_response/t/Raw", combinedData);
    //        SaveLoad::loadRawFromTimingData(saveFolderPath / "Key3_response/u/Raw", combinedData);
    //        SaveLoad::saveMetricsFromTimingData(saveFolderPath / "Key3_response/Combined",
    //                                            combinedData);
    //    }
    //    Experimental::correlate(saveFolderPath / "Key3_response/Correlate",
    //                            saveFolderPath / "Key3_response");

    //    Experimental::studyRun(saveFolderPath / "Key4_response/t", connectionKey, key4);
    //    Experimental::studyRun(saveFolderPath / "Key4_response/u", connectionKey, key4);
    //    {
    //        TimingData<true> combinedData{400, 2048 * 100, key4};
    //        SaveLoad::loadRawFromTimingData(saveFolderPath / "Key4_response/t/Raw", combinedData);
    //        SaveLoad::loadRawFromTimingData(saveFolderPath / "Key4_response/u/Raw", combinedData);
    //        SaveLoad::saveMetricsFromTimingData(saveFolderPath / "Key4_response/Combined",
    //                                            combinedData);
    //    }
    //    Experimental::correlate(saveFolderPath / "Key4_response/Correlate",
    //                            saveFolderPath / "Key4_response");

    //    Experimental::studyRun(saveFolderPath / "Key5_response/t", connectionKey, key5);
    //    Experimental::studyRun(saveFolderPath / "Key5_response/u", connectionKey, key5);
    //    {
    //        TimingData<true> combinedData{400, 2048 * 100, key5};
    //        SaveLoad::loadRawFromTimingData(saveFolderPath / "Key5_response/t/Raw", combinedData);
    //        SaveLoad::loadRawFromTimingData(saveFolderPath / "Key5_response/u/Raw", combinedData);
    //        SaveLoad::saveMetricsFromTimingData(saveFolderPath / "Key5_response/Combined",
    //                                            combinedData);
    //    }
    //    Experimental::correlate(saveFolderPath / "Key5_response/Correlate",
    //                            saveFolderPath / "Key5_response");

    //    Experimental::studyRun(saveFolderPath / "Key6_response/t", connectionKey, key6);
    //    Experimental::studyRun(saveFolderPath / "Key6_response/u", connectionKey, key6);
    //    {
    //        TimingData<true> combinedData{400, 2048 * 100, key6};
    //        SaveLoad::loadRawFromTimingData(saveFolderPath / "Key6_response/t/Raw", combinedData);
    //        SaveLoad::loadRawFromTimingData(saveFolderPath / "Key6_response/u/Raw", combinedData);
    //        SaveLoad::saveMetricsFromTimingData(saveFolderPath / "Key6_response/Combined",
    //                                            combinedData);
    //    }
    //    Experimental::correlate(saveFolderPath / "Key6_response/Correlate",
    //                            saveFolderPath / "Key6_response");
    //
    std::cout << "Exiting..." << std::endl;
    return 0;
}
