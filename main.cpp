#include "Correlate/Correlate.hpp"
#include "DataProcessing/Samples/SampleGroup.hpp"
#include "DataProcessing/Timings/TimingProcessing.hpp"
#include "ServerConnection/ServerConnection.hpp"
#include "Study/Study.hpp"
#include "Study/StudyDoppelganger.hpp"

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
    ServerKeyConnection connection1{ip, port};
    ServerKeyConnection connection2{ip, port};
    ServerKeyConnection connection3{ip, port};

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
    StudyDoppelganger study1{std::move(connection1), g_continueRunning, dataPacketLength};
    study1.loadRawStudyData("../../DATA/REMOTE_NEW_KEY_1/Raw");

    StudyDoppelganger study2{std::move(connection2), g_continueRunning, dataPacketLength};
    study2.loadRawStudyData("../../DATA/REMOTE_OLD_KEY_1/Raw");

    StudyDoppelganger study3{std::move(connection2), g_continueRunning, dataPacketLength};
    study3.loadRawStudyData("../../DATA/REMOTE_NEW_KEY_2/Raw");

    std::cout << "Different Key:\n";
    for (unsigned i{0}; i < Study::AES_BLOCK_SIZE; ++i)
    {
        double result1 = Correlate::computeFactorStdSampleGroup(study1.data()[i], study2.data()[i]);
        double result2 = Correlate::computeFactorSampleGroup(study1.data()[i], study2.data()[i]);
        std::cout << i << ":\t" << result1 << '\t' << result2 << '\n';
    }
    std::cout << "Final factor:\t"
              << Correlate::computeFactorStdSampleBlock(study1.data(), study2.data()) << '\t'
              << Correlate::computeFactorSampleBlock(study1.data(), study2.data()) << "\n\n";

    std::cout << "Same Key:\n";
    for (unsigned i{0}; i < Study::AES_BLOCK_SIZE; ++i)
    {
        double result1 = Correlate::computeFactorStdSampleGroup(study1.data()[i], study3.data()[i]);
        double result2 = Correlate::computeFactorSampleGroup(study1.data()[i], study3.data()[i]);
        std::cout << i << ":\t" << result1 << '\t' << result2 << '\n';
    }
    std::cout << "Final factor:\t"
              << Correlate::computeFactorStdSampleBlock(study1.data(), study3.data()) << '\t'
              << Correlate::computeFactorSampleBlock(study1.data(), study3.data()) << "\n\n";
    return 0;
}
