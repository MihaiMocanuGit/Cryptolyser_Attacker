#include "DataProcessing/Samples/SampleGroup.hpp"
#include "DataProcessing/Timings/TimingProcessing.hpp"
#include "ServerConnection/ServerConnection.hpp"
#include "Study/Study.hpp"

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

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Incorrect program parameters: <IPv4> <PORT> <SAVE_FOLDER_PATH>" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Starting..." << std::endl;
    const std::string saveFolderPath{argv[3]};
    std::filesystem::create_directory(saveFolderPath);
    std::cout << "Created directory: " + saveFolderPath << std::endl;

    const std::string_view ip{argv[1]};
    const uint16_t port{static_cast<uint16_t>(std::stoi(argv[2]))};
    ServerConnection connection{ip, port};

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

    constexpr size_t dataPacketLength = 512;
    constexpr size_t desiredAvgSampleSize = 4048 * 4;
    const Study::DisplayParams display{
        .printFreq = 200'000,
        .saveFreq = 2'000'000,
        .savePath = saveFolderPath,
    };

    Study study{std::move(connection), g_continueRunning, dataPacketLength};
    std::cout << "Calibrating bounds..." << std::endl;
    Study::TimingBoundaryParams computedBd = study.calibrate(saveFolderPath);
    std::cout << "Computed bounds: " << computedBd.lb << ", " << computedBd.ub << std::endl;
    std::cout << "Starting study..." << std::endl;

    study.start(desiredAvgSampleSize, display, computedBd);

    std::cout << "Exiting..." << std::endl;
    return 0;
}
