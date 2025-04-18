#include "Correlate/Correlate.hpp"
#include "DataProcessing/DataVector/DataVectorSerializer.hpp"
#include "DataProcessing/SampleData/SampleData.hpp"
#include "DataProcessing/SampleData/SampleDataSerializer.hpp"
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
volatile sig_atomic_t g_continueRunning {true};
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
constexpr size_t DESIRED_COUNT {2 * 32 * 1024 * 1024};
constexpr size_t LOG_FREQ {2 * 128 * 1024};
constexpr size_t SAVE_FREQ {2 * 3 * 1024 * 1024};
constexpr size_t CALIBRATE_COUNT {100'000};
constexpr double LB_CONFIDENCE {0.0};
constexpr double UB_CONFIDENCE {0.005};
constexpr size_t DATA_SIZE {800};
constexpr size_t RESERVE = 1.1 * DESIRED_COUNT / (AES_BLOCK_BYTE_SIZE * 256);

TimingData<false> studyRun(const std::filesystem::path &saveFolderPath,
                           ServerConnection<false> &connectionKeyless)
{

    TimingData<false> dataKeyless {DATA_SIZE};
    dataKeyless.reserveForEach(RESERVE);

    Gatherer<false> gathererKeyless {std::move(connectionKeyless), std::move(dataKeyless)};
    Study<false> studyKeyless {std::move(gathererKeyless), g_continueRunning, saveFolderPath};

    std::cout << "Started calibration..." << std::endl;
    auto [lb, ub] = studyKeyless.calibrateBounds(CALIBRATE_COUNT, LB_CONFIDENCE, UB_CONFIDENCE);
    std::cout << "Computed bounds: [" << lb << " " << ub << "]" << std::endl;

    std::cout << "Started run..." << std::endl;
    studyKeyless.run(DESIRED_COUNT, LOG_FREQ, SAVE_FREQ, lb, ub);
    std::cout << "Finished." << std::endl;

    gathererKeyless = studyKeyless.release();
    connectionKeyless = std::move(gathererKeyless.release().connection);
    return dataKeyless;
}

TimingData<true> studyRun(const std::filesystem::path &saveFolderPath,
                          ServerConnection<true> &connectionKey,
                          const std::array<std::byte, 16> &key)
{

    TimingData<true> dataKey {DATA_SIZE, key};
    dataKey.reserveForEach(RESERVE);

    Gatherer<true> gathererKey {std::move(connectionKey), std::move(dataKey)};
    Study<true> studyKey {std::move(gathererKey), g_continueRunning, saveFolderPath};

    std::cout << "Started calibration..." << std::endl;
    auto [lb, ub] = studyKey.calibrateBounds(CALIBRATE_COUNT, LB_CONFIDENCE, UB_CONFIDENCE);
    std::cout << "Computed bounds: [" << lb << " " << ub << "]" << std::endl;
    std::cout << "Started run..." << std::endl;
    studyKey.run(DESIRED_COUNT, LOG_FREQ, SAVE_FREQ, 0, ub);
    std::cout << "Finished." << std::endl;

    gathererKey = studyKey.release();
    connectionKey = std::move(gathererKey.release().connection);

    return dataKey;
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
    const std::filesystem::path saveFolderPath {argv[3]};

    const std::string_view ip {argv[1]};
    const uint16_t port {static_cast<uint16_t>(std::stoi(argv[2]))};

    struct sigaction sigactionExit {};
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
    // As the app does not have any implicit way of scheduling its jobs during this development
    // phase, the main.cpp file is actively used to experiment with different activities. Thus, this
    // file might be left in a disorganized state between the seldom commits.
    //
    // At this moment, the app can do the following jobs:
    // 1. Study Session: Using the Study class, connect to either Cryptolyser_Victim or
    // Cryptolyser_Doppelganger and request a study set.
    // 2. Save The obtained data to file. You can save the whole data, or save just the metrics
    // (size, mean, variance, min, max) value
    // 3. Load data that was previously stored in files.
    // 4. Correlate data - encrypted with a secret key - with another set with a known key.
    // 5. Generate the distribution of values from a data set.
    //
    // To be added:
    // i. Post-process the data through filtering.

    auto key1 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
    auto key2 = std::bit_cast<std::array<std::byte, 16>>(std::to_array<uint8_t>(
        {255, 254, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241}));
    auto key3 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 7}));
    auto key4 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));
    auto key5 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
                                0xF0, 0xF0, 0xF0, 0xF0, 0xF0}));
    auto key6 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
                                0x0F, 0x0F, 0x0F, 0x0F, 0x0F}));
    auto key7 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0x13, 0x67, 0x73, 0x1f, 0x26, 0x7f, 0x64, 0xfc, 0x42, 0x0f, 0x59,
                                0x62, 0x76, 0x6c, 0xf8, 0x68}));
    auto key8 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                0xff, 0xff, 0xff, 0xff, 0xff}));
    auto key9 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26,
                                0x26, 0x26, 0x26, 0x26, 0x26}));

    // ServerConnection<true> connectionKey(ip, port);
    // ServerConnection<false> connectionKeyless(ip, port);

    std::cout << "Exiting..." << std::endl;
    return 0;
}
