#include <csignal>
#include <filesystem>
#include <iostream>

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

int main(int argc, char **argv)
{
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
    // 6. Post-process the data through filtering.

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

    return 0;
}
