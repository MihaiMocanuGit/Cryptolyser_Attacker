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

    constexpr Study::Data data{
        .dataPacketLength = 512,
        .desiredMeanSampleSize = 4048 * 4,
    };
    const Study::Display display{
        .printFreq = 200'000,
        .saveFreq = 2'000'000,
        .savePath = saveFolderPath,
    };
    constexpr Study::TimingBoundary boundary{
        .lb = 25.0,
        .ub = 3000.0,
    };
    Study study{std::move(connection), g_continueRunning, data, display, boundary};
    study.start();

    std::cout << "Exiting..." << std::endl;
    return 0;
}
