#ifndef CRYPTOLYSER_ATTACKER_STUDY_HPP
#define CRYPTOLYSER_ATTACKER_STUDY_HPP

#include "DataProcessing/Samples/SampleGroup.hpp"
#include "ServerConnection/ServerConnection.hpp"

#include <csignal>
#include <fstream>
#include <random>

class Study
{
  public:
    struct DisplayParams
    {
        size_t printFreq;
        size_t saveFreq;
        std::string savePath;
    };

    struct TimingBoundaryParams
    {
        double lb;
        double ub;
    };
    static constexpr unsigned AES_BLOCK_SIZE{16};
    static constexpr unsigned SAMPLE_GROUP_SIZE{256};

    Study(ServerConnection &&connection, const volatile sig_atomic_t &continueRunningFlag,
          size_t dataPacketLength);
    Study(const Study &) = delete;
    Study &operator=(const Study &) = delete;

    TimingBoundaryParams calibrate(size_t transmissions);
    void start(size_t desiredAvgSampleSize, const DisplayParams &display,
               const TimingBoundaryParams &bounds);

    void loadPreviousStudyData(const std::string &prevRawDir);

    static void saveDataRaw(const std::string directory,
                            const std::vector<SampleGroup<double>> &data);
    static void saveDataMetrics(const std::string directoryName,
                                const std::vector<SampleGroup<double>> &data);
    const std::vector<SampleGroup<double>> &data() const;

  private:
    const size_t m_DATA_PACKET_LENGTH;
    ServerConnection m_connection;
    const volatile sig_atomic_t &m_continueRunning;
    std::vector<SampleGroup<double>> m_sampleGroups{AES_BLOCK_SIZE,
                                                    SampleGroup<double>{SAMPLE_GROUP_SIZE}};
    struct StudyContext;
    size_t m_totalCount(const StudyContext &ctx) const;
    bool filterCurrentValue(StudyContext &ctx, double timing);
    void printStats(StudyContext &ctx);

    bool isSaveTime(const StudyContext &ctx) const;
};

#endif // CRYPTOLYSER_ATTACKER_STUDY_HPP
