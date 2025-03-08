#ifndef CRYPTOLYSER_ATTACKER_STUDY_HPP
#define CRYPTOLYSER_ATTACKER_STUDY_HPP

#include "DataProcessing/Samples/SampleGroup.hpp"
#include "ServerConnection/ServerConnection.hpp"

#include <csignal>
#include <fstream>
#include <random>

class Study
{
    static constexpr unsigned m_AES_BLOCK_SIZE{16};
    static constexpr unsigned m_SAMPLE_GROUP_SIZE{256};

    const size_t m_DESIRED_MEAN_SAMPLE_SIZE;
    const size_t m_DATA_PACKET_LENGTH;
    ServerConnection m_connection;
    const volatile sig_atomic_t &m_continueRunning;
    const size_t m_APPROXIMATE_TOTAL_COUNT{m_AES_BLOCK_SIZE * m_SAMPLE_GROUP_SIZE *
                                           m_DESIRED_MEAN_SAMPLE_SIZE};
    std::vector<SampleGroup<double>> m_sampleGroups{
        m_AES_BLOCK_SIZE, SampleGroup<double>{m_SAMPLE_GROUP_SIZE, m_DESIRED_MEAN_SAMPLE_SIZE}};

    size_t m_currentCount{0};
    size_t m_currentId{0};
    size_t m_lostNetworkPackages{0};
    size_t m_ignoredValues{0};

    timespec m_startStudyTime{};
    timespec m_prevPassTime{.tv_sec = 0, .tv_nsec = 0};
    size_t m_prevPassPacketCount{0};

    const std::string m_SAVE_FOLDER_PATH;
    const size_t m_PRINT_FREQ;
    const size_t m_SAVE_FREQ;
    const double m_TIMING_LB;
    SampleData<double> m_sampleLB;
    const double m_TIMING_UB;
    SampleData<double> m_sampleUB;

    [[nodiscard]] size_t m_totalCount() const;
    bool filterCurrentValue(double timing);
    void printStats();
    void saveMetrics() const;
    void saveRaw() const;

  public:
    struct Data
    {
        size_t dataPacketLength;
        size_t desiredMeanSampleSize;
    };

    struct Display
    {
        size_t printFreq;
        size_t saveFreq;
        std::string savePath;
    };

    struct TimingBoundary
    {
        double lb;
        double ub;
    };

    Study(ServerConnection &&connection, const volatile sig_atomic_t &continueRunningFlag,
          const Data &data, const Display &display, const TimingBoundary &bounds);

    void start();
};

#endif // CRYPTOLYSER_ATTACKER_STUDY_HPP
