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

void studyRun(const std::filesystem::path &saveFolderPath, ServerConnection<true> &connectionKey,
              const std::array<std::byte, 16> &key1)

{
    constexpr size_t DATA_SIZE = 400;
    constexpr size_t RESERVE = 1024 * 1024;

    TimingData<true> dataKey{DATA_SIZE, RESERVE, key1};
    Gatherer<true> gathererKey{std::move(connectionKey), std::move(dataKey)};
    Study<true> studyKey{std::move(gathererKey), g_continueRunning, saveFolderPath};

    std::cout << "Started calibration..." << std::endl;
    auto [lb, ub] = studyKey.calibrateBounds();
    std::cout << "Computed bounds: [" << lb << " " << ub << "]" << std::endl;

    constexpr size_t DESIRED_COUNT = 64 * 1024 * 1024;
    constexpr size_t LOG_FREQ = 128 * 1024;
    constexpr size_t SAVE_FREQ = 4 * 1024 * 1024;

    std::cout << "Started run..." << std::endl;
    studyKey.run(DESIRED_COUNT, LOG_FREQ, SAVE_FREQ, lb, ub);
    std::cout << "Finished." << std::endl;

    gathererKey = std::move(studyKey.release());
    connectionKey = std::move(gathererKey.release().connection);
}
} // namespace

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

    ServerConnection<true> connectionKey(ip, port);

    auto key1 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}));
    auto key2 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0}));
    auto key3 = std::bit_cast<std::array<std::byte, 16>>(
        std::to_array<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 7}));

    studyRun(saveFolderPath / "Key1", connectionKey, key1);
    studyRun(saveFolderPath / "Key2", connectionKey, key2);
    studyRun(saveFolderPath / "Key3", connectionKey, key3);

    std::cout << "Exiting..." << std::endl;
    return 0;
}
